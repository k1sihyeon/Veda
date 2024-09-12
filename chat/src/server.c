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

#define LOGIN_FILE_DIR "/home/sihyeon/workspace/veda/chat/data/login.csv"

typedef struct message {
    int code;
    char id[20];
    char name[50];
    char buf[BUFSIZ];
} __attribute__((__packed__)) Msg;

typedef struct client {
    int sockfd;
    int pipe1[2];
    int pipe2[2];
    int child_pid;
} __attribute__((__packed__)) Client;

typedef struct user {
    char id[20];
    char name[50];
} __attribute__((__packed__)) User;

////    전역 변수    ////

Client clients[MAX_CLIENT];     // 사용 시 visibility 주의

////   함수 선언    ////
void siguser1(int signo);
void siguser2(int signo);
void sigchild(int signo);
void close_client(int idx);

Msg make_message(const int code, const char *id, const char *name, const char *buf);

int ServerSend(int sockfd, const char *buf);

int HelloWorld(int sockfd, sem_t *sem, User *user);
bool Login(int sockfd, sem_t *sem, User *user);
bool Resgister(int sockfd, sem_t *sem, User *user);

////    main 함수    ////

int main(int argc, char **argv) {
    pid_t pid;
    // socklen_t clen;
    struct sockaddr_in servaddr, cliaddr;
    FILE *fp;
    sem_t *sem;
    User *user;

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

    if (listen(ssock, MAX_CLIENT) < 0) {
        perror("listen");
        exit(1);
    }

    sem = sem_open("login", O_CREAT, 0644, 1);

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
        if ((pid = fork()) < 0) {  ///////////////////////////////////////
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {  // child
            close(ssock);
            // close(clients[client_idx].pipe[WRITE_FD]);
            clients[client_idx].child_pid = getpid();

            // signal
            signal(SIGUSR2, siguser2);

            // first screen
            user = (User *)malloc(sizeof(User));
            memset(user, 0, sizeof(User));
            int out;
            while (true) {
                out = HelloWorld(clients[client_idx].sockfd, sem, user);
                if (out == -1) {
                    close_client(client_idx);
                    goto CLOSE;
                    break;
                }

                if (out == true) {
                    break;
                }
            }

            if (user->id[0] == 0) {
                close_client(client_idx);
                goto CLOSE;
            }

            // -> 로그인 성공 이후
            // 클라이언트가 로그인 하기 전에도 입장 메시지가 나옴
            // 수정 필요

            // 채팅방 입장 메시지 전송
            Msg msg;
            msg = make_message(2, user->id, user->name, "");
            write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
            kill(getppid(), SIGUSR1);

            ServerSend(csock, "\033[2J\033[1;1H");
            ServerSend(csock, "===============================\n");
            ServerSend(csock, "     Welcome to chat server    \n");
            ServerSend(csock, "===============================\n");
            ServerSend(csock, "     Type '!exit' to exit      \n");
            ServerSend(csock, "===============================\n\n");

            // read socket & write pipe
            while (true) {
                char buf[BUFSIZ];

                if (read(clients[client_idx].sockfd, buf, sizeof(buf)) > 0) {
                    printf("child: %s\n", buf);

                    if (!strncmp(buf, "!exit", 5)) {
                        // 퇴장 메시지 전송
                        msg = make_message(3, user->id, user->name, "");
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);

                        msg = make_message(-1, "", "", "");
                        write(clients[client_idx].sockfd, &msg, sizeof(Msg));
                        break;
                    }

                    // 일반 메시지 처리
                    msg = make_message(1, user->id, user->name, buf);
                    write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                    kill(getppid(), SIGUSR1);
                }
            }

            close_client(client_idx);
        }
        else {  // parent
            // close(clients[client_idx].pipe[READ_FD]);
            clients[client_idx].child_pid = pid;

            // signal
            signal(SIGUSR1, siguser1);
            signal(SIGCHLD, sigchild);
        }
    }

CLOSE:;
    Msg msg = make_message(-1, "", "", "");
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i].sockfd != 0) {
            write(clients[i].pipe1[WRITE_FD], &msg, sizeof(Msg));
            kill(clients[i].child_pid, SIGUSR2);
            close_client(i);
        }
    }
    close(ssock);
    sem_close(sem);
    sem_unlink("login");


    return 0;
}

////  함수 정의    ////

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

// 서버에서 클라이언트로 메시지 전송
int ServerSend(int sockfd, const char *buf) {
    Msg msg = make_message(0, "", "", buf);
    return write(sockfd, &msg, sizeof(Msg));
}

// 첫 화면
int HelloWorld(int sockfd, sem_t *sem, User *user) {
    int num;
    char buf[BUFSIZ];

    ServerSend(sockfd, "\033[2J\033[1;1H");
    ServerSend(sockfd, "Welcome to chat server\n");
    ServerSend(sockfd, "=======================\n");
    ServerSend(sockfd, "===== Select Menu =====\n");
    ServerSend(sockfd, "=======================\n");
    ServerSend(sockfd, "1. Login\n");
    ServerSend(sockfd, "2. Register\n");
    ServerSend(sockfd, "3. Exit\n");
    ServerSend(sockfd, "=======================\n");
    ServerSend(sockfd, "Input >> ");

    read(sockfd, buf, sizeof(buf));
    num = atoi(buf);

    if (num == 1) {
        while(!Login(sockfd, sem, user)) {};
        return true;
    }
    else if (num == 2) {
        while(!Resgister(sockfd, sem, user)) {};
        return true;
    }
    else if (num == 3) {
        return -1;
    }
    else {
        ServerSend(sockfd, "Invalid input\n");
        return false;
    }

    return false;
}

// 로그인 함수
bool Login(int sockfd, sem_t *sem, User *user) {
    char buf[BUFSIZ], uid[20], upw[50];
    char fid[20], fpw[50], fname[50];
    int n;
    FILE *csv_fp;
    bool isLogin = false;

    ServerSend(sockfd, "\033[2J\033[1;1H");
    ServerSend(sockfd, "Input ID >> ");
    n = read(sockfd, buf, BUFSIZ);
    buf[n] = '\0';
    strncpy(uid, strtok(buf, "\n"), 20);
    memset(buf, 0, BUFSIZ);

    ServerSend(sockfd, "Input Password >> ");
    n = read(sockfd, buf, BUFSIZ);
    buf[n] = '\0';
    strncpy(upw, strtok(buf, "\n"), 50);
    memset(buf, 0, BUFSIZ);

    // csv로 로그인 유효성 검사
    sem_wait(sem);
    csv_fp = fopen(LOGIN_FILE_DIR, "r");
    while (fgets(buf, BUFSIZ, csv_fp) != NULL) {
        sscanf(buf, "%[^,], %[^,], %s", fid, fpw, fname);

        if (!strcmp(uid, fid) && !strcmp(upw, fpw)) {
            isLogin = true;
            break;
        }
    }
    
    fclose(csv_fp);
    sem_post(sem);

    if (isLogin) {
        strcpy(user->id, uid);
        strcpy(user->name, fname);
        ServerSend(sockfd, "Login success\n");
        sleep(1);
        return true;
    }
    else {
        ServerSend(sockfd, "Login failed\n");
        sleep(1);
        return false;
    }

    return true;
}

// 회원가입 함수
bool Resgister(int sockfd, sem_t *sem, User *user) {
    char buf[BUFSIZ];
    char id[20], name[50], pw[50];
    int n;
    FILE *csv_fp;

    ServerSend(sockfd, "\033[2J\033[1;1H");
    ServerSend(sockfd, "Input ID >> ");
    n = read(sockfd, buf, BUFSIZ);
    buf[n] = '\0';
    strncpy(id, strtok(buf, "\n"), 20);
    memset(buf, 0, BUFSIZ);

    ServerSend(sockfd, "Input Name >> ");
    n = read(sockfd, buf, BUFSIZ);
    buf[n] = '\0';
    strncpy(name, strtok(buf, "\n"), 50);
    memset(buf, 0, BUFSIZ);

    ServerSend(sockfd, "Input Password >> ");
    n = read(sockfd, buf, BUFSIZ);
    buf[n] = '\0';
    strncpy(pw, strtok(buf, "\n"), 50);
    memset(buf, 0, BUFSIZ);

    // file semaphore
    sem_wait(sem);

    csv_fp = fopen(LOGIN_FILE_DIR, "a");
    fprintf(csv_fp, "%s, %s, %s\n", id, pw, name);
    fclose(csv_fp);

    sem_post(sem);

    strcpy(user->id, id);
    strcpy(user->name, name);

    ServerSend(sockfd, "Register success\n");
    sleep(1);

    return true;
}