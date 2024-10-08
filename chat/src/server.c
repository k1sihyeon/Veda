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

#define TCP_PORT    5100
#define MAX_CLIENT  100

#define READ_FD     0
#define WRITE_FD    1

#define LOGIN_FILE_DIR "../data/login.csv"

#define DISCON_SERVER_CODE  -1
#define SERVER_MSG_CODE     0
#define CLIENT_MSG_CODE     1
#define ENTER_MSG_CODE      2
#define EXIT_MSG_CODE       3
#define LIST_MSG_CODE       4
#define LIST_MSG_CODE2      5
#define WHISPER_MSG_CODE    6
#define USER_INFO_CODE      7
#define FILE_SEND_CODE      8
#define FILE_RECV_CODE      9

typedef struct message {
    int  code;
    char id[20];
    char name[50];
    int  group;
    char buf[BUFSIZ];
    char destID[20];
    char filename[100];
    int  filesize;
} __attribute__((__packed__)) Msg;

typedef struct user {
    char id[20];
    char name[50];
    int  group;
} __attribute__((__packed__)) User;

typedef struct client {
    int sockfd;
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];
    int child_pid;
    User user;
} __attribute__((__packed__)) Client;

////    전역 변수    ////

Client clients[MAX_CLIENT];     // 사용 시 visibility 주의

////   함수 선언    ////
void siguser1(int signo);
void siguser2(int signo);
void sigchild(int signo);
void close_client(int idx);

Msg make_message(const int code, const char *id, const char *name, const int group, const char *buf);

int ServerSend(int sockfd, const char *buf);

int HelloWorld(int sockfd, sem_t *sem, User *user);
bool Login(int sockfd, sem_t *sem, User *user);
bool Resgister(int sockfd, sem_t *sem, User *user);
void EnterChatRoom(int sockfd, int group);

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
                if (pipe(clients[i].pipe1) < 0 || pipe(clients[i].pipe2) < 0 || pipe(clients[i].pipe3) < 0) {
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
        fcntl(clients[client_idx].pipe3[READ_FD], F_SETFL, O_NONBLOCK);
        fcntl(clients[client_idx].pipe3[WRITE_FD], F_SETFL, O_NONBLOCK);

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
                    Msg emsg = make_message(DISCON_SERVER_CODE, "", "", 0, "");
                    write(csock, &emsg, sizeof(Msg));
                    close_client(client_idx);
                    printf("client disconnected\n");
                    goto CLOSE;
                    break;
                }

                if (out == true) {
                    //printf("login success\n");
                    break;
                }

                if (out == false) {
                    //printf("login failed\n");
                    continue;
                }
            }

            if (user->id[0] == 0) {
                close_client(client_idx);
                goto CLOSE;
            }

            // 기본 그룹 0
            user->group = 0;

            strcpy(clients[client_idx].user.id, user->id);
            strcpy(clients[client_idx].user.name, user->name);
            clients[client_idx].user.group = user->group;

            // 유저 정보 전송
            Msg msg;
            msg = make_message(7, user->id, user->name, user->group, "");
            write(csock, &msg, sizeof(Msg));

            // -> 로그인 성공 이후
            // 클라이언트가 로그인 하기 전에도 입장 메시지가 나옴
            // 수정 필요

            // 채팅방 입장 메시지 전송
            msg = make_message(2, user->id, user->name, user->group, "");
            write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
            kill(getppid(), SIGUSR1);

            EnterChatRoom(csock, user->group);

            // read socket & write pipe
            while (true) {
                char buf[BUFSIZ];

                if (read(clients[client_idx].sockfd, buf, sizeof(buf)) > 0) {
                    printf("child: %s\n", buf);

                    // !help
                    if (!strncmp(buf, "!help", 5)) {
                        ServerSend(csock, "----------------------------------------------------------\n");
                        ServerSend(csock, " '!help'                     : show help message \n");
                        ServerSend(csock, " '!exit', '!quit', '!q'      : close connection and exit \n");
                        ServerSend(csock, " '!list', '!users'           : show user list \n");
                        ServerSend(csock, " '!group [number]'           : change group \n");
                        ServerSend(csock, " '!announce [message]'       : send announcement \n");
                        ServerSend(csock, " '!whisper [id] [message]'   : whisper to user \n");
                        ServerSend(csock, " '@ [id] [message]'          : whisper to user \n");
                        ServerSend(csock, " '!color [color] [message]'  : change text color \n");
                        ServerSend(csock, "         [color]             : 30 ~ 37 \n");
                        ServerSend(csock, " '!send [file1] [file2]      : send file to server\n");
                        ServerSend(csock, " '!recv [file1] [file2]      : receive file from server\n");
                        ServerSend(csock, " '!clear', '!cls'            : clear screen \n");
                        ServerSend(csock, "----------------------------------------------------------\n");

                        continue;
                    }

                    // !exit, !quit, !q
                    if (!strncmp(buf, "!exit", 5) || !strncmp(buf, "!quit", 5) || !strncmp(buf, "!q", 2)) {
                        // 퇴장 메시지 broadcast
                        msg = make_message(3, user->id, user->name, user->group, "");
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);

                        // 종료 메시지 전송
                        msg = make_message(DISCON_SERVER_CODE, "", "", 0, "");
                        write(clients[client_idx].sockfd, &msg, sizeof(Msg));
                        break;
                    }

                    // !list, !users
                    if (!strncmp(buf, "!list", 5) || !strncmp(buf, "!users", 6)) {
                        msg = make_message(LIST_MSG_CODE, user->id, user->name, user->group, "");
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);
                        continue;
                    }

                    // !group
                    if (!strncmp(buf, "!group", 6)) {
                        char *ptr = strtok(buf, " ");
                        char *group = strtok(NULL, "");

                        if (group == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }
                        
                        // 기존 그룹 퇴장 메시지 broadcast
                        msg = make_message(EXIT_MSG_CODE, user->id, user->name, user->group, "");
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);

                        // 새로운 그룹으로 변경
                        user->group = atoi(group);
                        
                        // 유저 정보 전송
                        msg = make_message(USER_INFO_CODE, user->id, user->name, user->group, "");
                        write(csock, &msg, sizeof(Msg));

                        EnterChatRoom(csock, user->group);

                        // 새로운 그룹 입장 메시지 broadcast
                        msg = make_message(ENTER_MSG_CODE, user->id, user->name, user->group, "");
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);
                        
                        // 그룹 바뀌는데 클라이언트 입력이 밀림..

                        continue;
                    }

                    // !announce
                    if (!strncmp(buf, "!announce", 9)) {
                        char *ptr = strtok(buf, " ");
                        char *message = strtok(NULL, "");

                        if (message == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }

                        char abuf[BUFSIZ];
                        sprintf(abuf, "\033[1;33m[Announcement] %s\033[0m", message);

                        msg = make_message(SERVER_MSG_CODE, "", "", "", abuf);
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);
                        
                        continue;
                    }
                    
                    // !whisper, @
                    if (!strncmp(buf, "!whisper", 8) || buf[0] == '@') {
                        char *ptr = strtok(buf, " ");
                        char *id = strtok(NULL, " ");
                        char *message = strtok(NULL, "");

                        if (id == NULL || message == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }

                        msg = make_message(WHISPER_MSG_CODE, user->id, user->name, user->group, message);
                        strcpy(msg.destID, id);
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);
                        
                        continue;
                    }

                    // !color
                    if (!strncmp(buf, "!color", 6)) {
                        // color change
                        char *ptr = strtok(buf, " ");
                        char *color = strtok(NULL, " ");
                        char *message = strtok(NULL, "");

                        if (color == NULL || message == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }

                        char color_buf[BUFSIZ];
                        sprintf(color_buf, "\033[1;%sm%s\033[0m", color, message);
                        msg = make_message(CLIENT_MSG_CODE, user->id, user->name, user->group, color_buf);
                        write(clients[client_idx].pipe1[WRITE_FD], &msg, sizeof(Msg));
                        kill(getppid(), SIGUSR1);
                        
                        continue;
                    }

                    // !send
                    if (!strncmp(buf, "!send", 5)) {
                        // 클라이언트에서 파일 전송 -> 서버에서 파일 수신
                        char *ptr = strtok(buf, " ");
                        char *file1 = strtok(NULL, " "); // in client
                        char *file2 = strtok(NULL, ""); // in server

                        if (file1 == NULL || file2 == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }

                        if (file1 != NULL) {
                            file1[strcspn(file1, "\n")] = '\0';
                        }
                        if (file2 != NULL) {
                            file2[strcspn(file2, "\n")] = '\0';
                        }

                        msg = make_message(FILE_SEND_CODE, user->id, user->name, user->group, file1);
                        write(csock, &msg, sizeof(Msg));

                        // 파일 청크 단위 수신
                        Msg fmsg;
                        memset(&fmsg, 0, sizeof(Msg));
                        strcpy(fmsg.filename, file1);
                        fmsg.filesize = 0;

                        FILE *file = fopen(file2, "wb");
                        if (file == NULL) {
                            printf("file open error in server\n");
                            sleep(1);
                            continue;
                        }

                        int received_bytes = 0;

                        // 파일 수신
                        do {
                            int nbytes = recv(csock, &msg, sizeof(Msg), 0);
                            if (nbytes <= 0) {
                                perror("recv()");
                                break;
                            }
                            fwrite(msg.buf, 1, nbytes, file);  // 실제 받은 바이트만큼 파일에 쓰기
                            received_bytes += nbytes;
                        } while (received_bytes < msg.filesize);

                        fclose(file);
                        printf("file received: %s\n", file1);

                        continue;
                    }

                    // !recv
                    if (!strncmp(buf, "!recv", 5)) {
                        // 서버에서 파일 전송 -> 클라이언트에서 파일 수신
                        char *ptr = strtok(buf, " ");
                        char *file1 = strtok(NULL, " "); // in server
                        char *file2 = strtok(NULL, ""); // in client

                        if (file1 == NULL || file2 == NULL) {
                            ServerSend(csock, "Invalid input\n");
                            sleep(1);
                            continue;
                        }

                        if (file1 != NULL) {
                            file1[strcspn(file1, "\n")] = '\0';
                        }
                        if (file2 != NULL) {
                            file2[strcspn(file2, "\n")] = '\0';
                        }

                        msg = make_message(FILE_RECV_CODE, user->id, user->name, user->group, file2);
                        write(csock, &msg, sizeof(Msg));

                        // 파일 청크 단위 전송
                        FILE *file = fopen(file1, "rb");
                        if (file == NULL) {
                            perror("file open error (recv in server)");
                            continue;
                        }

                        Msg fmsg;
                        memset(&fmsg, 0, sizeof(Msg));
                        strcpy(fmsg.filename, file2);
                        
                        fseek(file, 0, SEEK_END);
                        fmsg.filesize = ftell(file);
                        fseek(file, 0, SEEK_SET);

                        while (fread(fmsg.buf, 1, BUFSIZ, file) > 0) {
                            fmsg.code = FILE_RECV_CODE;
                            strcpy(fmsg.id, user->id);
                            strcpy(fmsg.name, user->name);
                            fmsg.group = user->group;

                            if (send(csock, &fmsg, sizeof(Msg), 0) <= 0) {
                                perror("send()");
                                fclose(file);
                                continue;
                            }
                        }

                        fclose(file);

                        continue;
                    }

                    // !clear, !cls
                    if (!strncmp(buf, "!clear", 6) || !strncmp(buf, "!cls", 4)) {
                        ServerSend(csock, "\033[2J\033[1;1H");
                        continue;
                    }

                    // 일반 메시지 처리
                    msg = make_message(CLIENT_MSG_CODE, user->id, user->name, user->group, buf);
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
    Msg msg = make_message(DISCON_SERVER_CODE, "", "", -1, "");
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

// 부모 프로세스는 파이프로 메시지를 받아 다른 자식 프로세스(sigusr2)에게 전달
void siguser1(int signo) {
    printf("SIGUSER1\n");

    Msg msg;
    Msg list_msg;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i].sockfd != 0) {
            if (read(clients[i].pipe1[READ_FD], &msg, sizeof(Msg)) > 0) {
                // broadcast
                for (int j = 0; j < MAX_CLIENT; j++) {
                    if (clients[j].sockfd != 0) {
                        write(clients[j].pipe2[WRITE_FD], &msg, sizeof(Msg));
                        kill(clients[j].child_pid, SIGUSR2);
                    }
                }
                return;
            }

            if (read(clients[i].pipe3[READ_FD], &list_msg, sizeof(Msg)) > 0) {
                if (list_msg.code == LIST_MSG_CODE) 
                    strcat(buf, list_msg.buf);
            }
        }
    }

    // list 메시지 전송
    if (buf[0] != 0) {
        list_msg = make_message(LIST_MSG_CODE2, list_msg.id, list_msg.name, list_msg.group, buf);
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (clients[i].sockfd != 0) {
                write(clients[i].pipe2[WRITE_FD], &list_msg, sizeof(Msg));
                kill(clients[i].child_pid, SIGUSR2);
            }
        }
    }

}

// 자식 프로세스는 파이프로 메시지를 받아 클라이언트(csock)에 전달
void siguser2(int signo) {
    printf("SIGUSER2\n");

    // 자식 프로세스의 clients 배열은 자신과 이전에 생성된 자식 프로세스의 정보만 가지고 있음
    // -> getpid()으로 자신의 pid를 찾아서 메시지를 전달해야 함

    Msg msg;
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i].child_pid == getpid()) {
            if (read(clients[i].pipe2[READ_FD], &msg, sizeof(Msg)) > 0) {

                if (msg.code == LIST_MSG_CODE) {
                    sprintf(msg.buf, "%s [%s]\n", clients[i].user.name, clients[i].user.id);
                    printf("list: %s", msg.buf);
                    write(clients[i].pipe3[WRITE_FD], &msg, sizeof(Msg));
                    kill(getppid(), SIGUSR1);
                    break;
                }
                // list 브로드 캐스팅 받으면 클라이언트에서 각각 name, id 보내고
                // 서버에서는 그걸 받아서 리스트 만들어서 보내주기

                if (msg.code == LIST_MSG_CODE2) {
                    if (strcmp(msg.id, clients[i].user.id) == 0) {
                        write(clients[i].sockfd, &msg, sizeof(Msg));
                        // 다른 클라이언트에서도 보임,,
                        // 왜지.. 감자..
                        break;
                    }
                }

                write(clients[i].sockfd, &msg, sizeof(Msg));
                break;
            }
            //break;
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
    close(clients[idx].pipe3[READ_FD]);
    close(clients[idx].pipe3[WRITE_FD]);

    clients[idx].sockfd = 0;
    clients[idx].child_pid = 0;
}

// 자식 프로레스 종료 시
void sigchild(int signo) {
    // ..?
}

// 메시지 생성 함수
Msg make_message(const int code, const char *id, const char *name, const int group, const char *buf) {
    Msg msg;
    msg.code = code;
    strcpy(msg.id, id);
    strcpy(msg.name, name);
    strcpy(msg.buf, buf);
    msg.group = group;
    return msg;
}

// 서버에서 클라이언트로 메시지 전송
int ServerSend(int sockfd, const char *buf) {
    Msg msg = make_message(0, "", "", 0, buf);
    return write(sockfd, &msg, sizeof(Msg));
}

// 첫 화면
int HelloWorld(int sockfd, sem_t *sem, User *user) {
    int num;
    char buf[BUFSIZ];

    ServerSend(sockfd, "\033[2J\033[1;1H");
    ServerSend(sockfd, "                  Welcome to chat server                 \n");
    ServerSend(sockfd, "==========================================================\n");
    ServerSend(sockfd, "                        Select Menu                       \n");
    ServerSend(sockfd, "==========================================================\n");
    ServerSend(sockfd, "    1. Login\n");
    ServerSend(sockfd, "    2. Register\n");
    ServerSend(sockfd, "    3. Exit\n");
    ServerSend(sockfd, "==========================================================\n");
    ServerSend(sockfd, "    Input Number >> ");

    read(sockfd, buf, sizeof(buf));
    num = atoi(buf);

    if (num == 1) {
        if (Login(sockfd, sem, user))
            return true;
        else
            return false;
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
        sleep(1);
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
    if (csv_fp == NULL) {
        ServerSend(sockfd, "Login File Does NOT exsit, DO REGISTER\n");
        sleep(1);
        return false;
    }

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

void EnterChatRoom(int sockfd, int group) {
    char buf[BUFSIZ];
    sprintf(buf, "             Welcome to chat server (group %d)           \n", group);

    ServerSend(sockfd, "\033[2J\033[1;1H");
    ServerSend(sockfd, "==========================================================\n");
    ServerSend(sockfd, buf);
    ServerSend(sockfd, "==========================================================\n");
    ServerSend(sockfd, "                   Type '!exit' to exit                   \n");
    ServerSend(sockfd, "            Type '!help' to get more commands             \n");
    ServerSend(sockfd, "==========================================================\n\n");
}