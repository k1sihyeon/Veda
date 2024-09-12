#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define TCP_PORT 5100
#define MAX_CLIENT 10

#define READ_FD 0
#define WRITE_FD 1

typedef struct message {
    int code;
    char id[20];
    char name[20];
    char buf[BUFSIZ];
} __attribute__((__packed__)) Msg;

typedef struct client {
    int sockfd;
    int pipe1[2];
    int pipe2[2];
    int child_pid;
} __attribute__((__packed__)) Client;

////    전역 변수    ////

Client clients[MAX_CLIENT];

////   함수 선언    ////

// 부모 프로세스에서 파이프로 메시지를 받아 다른 자식 프로세스(sigusr2)에게 전달
void siguser1(int signo) {
    printf("SIGUSER1\n");

    Msg msg;
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i].sockfd != 0) {
            if (read(clients[i].pipe1[READ_FD], &msg, sizeof(Msg)) > 0) {
                for (int j = 0; j < MAX_CLIENT; j++) {
                    if (clients[j].sockfd != 0) {
                        write(clients[j].pipe2[WRITE_FD], &msg, sizeof(Msg));
                        kill(clients[j].child_pid, SIGUSR2);
                    }
                }
            }
        }
    }
}

// 자식 프로세스에서 파이프로 메시지를 받아 클라이언트(csock)에 전달
void siguser2(int signo) {
    printf("SIGUSER2\n");

    // 자식 프로세스의 clients 배열은 자신과 이전에 생성된 자식 프로세스의 정보만 가지고 있음
    // -> getpid()으로 자신의 pid를 찾아서 메시지를 전달해야 함 

    Msg msg;
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i].child_pid == getpid()) {
            if (read(clients[i].pipe2[READ_FD], &msg, sizeof(Msg)) > 0) {
                write(clients[i].sockfd, &msg, sizeof(Msg));
            }
            break;
        }
    }
}

// 클라이언트 연결 종료 시
void close_client(int idx) {
    close(clients[idx].sockfd);
    close(clients[idx].pipe1[READ_FD]);
    close(clients[idx].pipe1[WRITE_FD]);
    close(clients[idx].pipe2[READ_FD]);
    close(clients[idx].pipe2[WRITE_FD]);
    clients[idx].sockfd = 0;
    clients[idx].child_pid = 0;
}

// 자식 프로레스 종료 시
void sigchild(int signo) {
    // ..?
}


// 메시지 생성 함수
Msg make_message(const int code, const char *id, const char *name, const char *buf) {
    Msg msg;
    msg.code = code;
    strcpy(msg.id, id);
    strcpy(msg.name, name);
    strcpy(msg.buf, buf);
    return msg;
}

int main(int argc, char **argv) {
    pid_t pid;
    // socklen_t clen;
    struct sockaddr_in servaddr, cliaddr;
    FILE *fp;
    sem_t *sem;

    int ssock, csock, port = TCP_PORT;

    ///////

    if (argc == 2)
        port = atoi(argv[1]);

    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(ssock, 5) < 0) {
        perror("listen");
        exit(1);
    }

    sem = sem_open("login", O_CREAT, 0600, 1);

    memset(clients, 0, sizeof(clients));

    while (true) {
        char client_ip[BUFSIZ];
        int client_port, clen, client_idx = -1;

        // client 연결됨
        clen = sizeof(cliaddr);
        if ((csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen)) < 0) {
            perror("accept");
            exit(1);
        }
        inet_ntop(AF_INET, &cliaddr.sin_addr, client_ip, sizeof(client_ip));
        client_port = ntohs(cliaddr.sin_port);
        printf("client connected: %s:%d\n", client_ip, client_port);

        // find empty slot
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (clients[i].sockfd == 0) {
                client_idx = i;

                clients[i].sockfd = csock;
                // pipe
                if (pipe(clients[i].pipe1) < 0 || pipe(clients[i].pipe2) < 0) {
                    perror("pipe");
                    exit(1);
                }

                break;
            }
        }

        if (client_idx == -1) {
            printf("too many clients\n");
            // send
            close(csock);
            continue;
        }

        // pipe non-blocking
        fcntl(clients[client_idx].pipe1[READ_FD], F_SETFL, O_NONBLOCK);
        fcntl(clients[client_idx].pipe1[WRITE_FD], F_SETFL, O_NONBLOCK);
        fcntl(clients[client_idx].pipe2[READ_FD], F_SETFL, O_NONBLOCK);
        fcntl(clients[client_idx].pipe2[WRITE_FD], F_SETFL, O_NONBLOCK);


        // fork
        if ((pid = fork()) < 0) {       ///////////////////////////////////////
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {    // child
            close(ssock);
            // close(clients[client_idx].pipe[WRITE_FD]);
            clients[client_idx].child_pid = getpid();

            // signal
            signal(SIGUSR2, siguser2);

            // read socket & write pipe
            Msg msg;
            while (true) {
                char buf[BUFSIZ];

                if (read(clients[client_idx].sockfd, buf, sizeof(buf)) > 0) {
                    printf("child: %s\n", buf);
                    msg = make_message(0, "id", "name", buf);
                    write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                    kill(getppid(), SIGUSR1);
                }
            }

            close_client(client_idx);
        }
        else {              // parent
            //close(clients[client_idx].pipe[READ_FD]);
            clients[client_idx].child_pid = pid;

            // signal
            signal(SIGUSR1, siguser1);
            signal(SIGCHLD, sigchild);
            
        }
    }

    return 0;
}
