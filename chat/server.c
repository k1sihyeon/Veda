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
#include <unistd.h>

#define TCP_PORT 5100
#define MAX_CLIENT 10

static int CHILD_COUNT = 0;

// client
// 1. 사용자 지정 이름, 색상 설정
// 2. 메시지 입력, 전송
// 3. 다른 client 메시지 수신, 출력 -> fork

typedef struct user {
    char id[BUFSIZ];
    char name[BUFSIZ];
} User;

typedef struct message {
    int code;
    char id[BUFSIZ];
    char name[BUFSIZ];
    char mesg[BUFSIZ];
} Message;

void sigfunc(int no) {
    printf("Signal from child(%d) : %d\n", CHILD_COUNT, no);
    CHILD_COUNT -= 1;

    if (CHILD_COUNT == 0)
        exit(0);
}

void Send(int sockfd, const char *buf, User* user/*, int code*/) {
    Message* msg = (Message*)malloc(sizeof(Message));
    memset(msg, 0, sizeof(Message));
    memset(msg->id, 0, BUFSIZ);
    memset(msg->name, 0, BUFSIZ);
    memset(msg->mesg, 0, BUFSIZ);

    strcpy(msg->mesg, buf);
    msg->code = 0;

    if (user == NULL) {
        strcpy(msg->id, "server");
        strcpy(msg->name, "server");
    }
    else {
        strcpy(msg->id, user->id);
        strcpy(msg->name, user->name);
    }

    size_t n = send(sockfd, msg, sizeof(Message), MSG_DONTWAIT);
    usleep(100);
    free(msg);

    if (n <= 0) {
        perror("Send()");
        exit(0);
    }

    //printf("Send data : %s", (char*)buf);
}

int main(int argc, char **argv) {
    pid_t pid;
    socklen_t clen;
    struct sockaddr_in servaddr, cliaddr;

    FILE *csv_fp;
    sem_t *sem;

    int n;
    int port = TCP_PORT;
    int ssock;

    char mesg[BUFSIZ];

    //////////////////////////////

    sem = sem_open("login_sem", O_CREAT, 0644, 1);

    if (argc == 2) {
        port = atoi(argv[1]);
    }

    // child proc. signal 처리 설정
    signal(SIGCHLD, sigfunc);

    // server tcp socket 생성
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind()");
        return -1;
    }

    if (listen(ssock, MAX_CLIENT) < 0) {
        perror("listen()");
        return -1;
    }

    clen = sizeof(cliaddr);

    do {
        int n;
        char ip[BUFSIZ];
        int csock;

        User user;
        memset(&user, 0, sizeof(User));

        clen = sizeof(cliaddr);
        csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
        inet_ntop(AF_INET, &cliaddr.sin_addr, ip, BUFSIZ);
        printf("Client is connected : %s\n", ip);
        CHILD_COUNT += 1;

        if ((pid = fork()) < 0)  ///////////////////////  fork  //////////////////////////////
            perror("fork()");
        else if (pid == 0) {  // Child Proc. - 클라이언트가 연결될 때 마다
            close(ssock);

            // 로그인, 사용자 이름 설정
            while (true) {
                int num;
                char buf[BUFSIZ];

                Send(csock, "\033[2J\033[1;1H", NULL);
                Send(csock, "Welcome to chat server\n", NULL);
                Send(csock, "=======================\n", NULL);
                Send(csock, "===== Select Menu =====\n", NULL);
                Send(csock, "=======================\n", NULL);
                Send(csock, "1. Login\n", NULL);
                Send(csock, "2. Register\n", NULL);
                Send(csock, "3. Exit\n", NULL);
                Send(csock, "=======================\n", NULL);
                Send(csock, "Input >> ", NULL);

                read(csock, buf, BUFSIZ);
                num = atoi(buf);

                if (num == 1) {
                    char uid[BUFSIZ];
                    char upw[BUFSIZ];

                    char fid[BUFSIZ];
                    char fpw[BUFSIZ];
                    char fname[BUFSIZ];

                    bool isLogin = false;

                    Send(csock, "\033[2J\033[1;1H", NULL);
                    Send(csock, "Input ID >> ", NULL);
                    n = read(csock, uid, BUFSIZ);
                    uid[n] = '\0';
                    strcpy(strtok(uid, "\n"), uid);

                    Send(csock, "Input Password >> ", NULL);
                    n = read(csock, upw, BUFSIZ);
                    upw[n] = '\0';
                    strcpy(strtok(upw, "\n"), upw);

                    // csv 파일(세마포어 필요)로 로그인 유효성 검사
                    sem_wait(sem);
                    csv_fp = fopen("login.csv", "r");

                    // 사용자 정보 검색
                    while (fscanf(csv_fp, "%s, %s, %s\n", fid, fpw, fname) != EOF) {
                        if (!strcmp(uid, fid) && !strcmp(upw, fpw)) {
                            isLogin = true;
                            break;
                        }
                    }
                    fclose(csv_fp);
                    sem_post(sem);

                    if (isLogin) {
                        Send(csock, "Login Success !!\n", NULL);
                        strcpy(user.id, uid);
                        strcpy(user.name, fname);
                        break;
                    }
                    else {
                        Send(csock, "Login Failed !!\n", NULL);
                        continue;
                    }
                }
                else if (num == 2) {
                    char buf[BUFSIZ];

                    char uid[BUFSIZ];
                    char upw[BUFSIZ];
                    char uname[BUFSIZ];

                    Send(csock, "\033[2J\033[1;1H", NULL);
                    Send(csock, "Input ID >> ", NULL);
                    n = read(csock, buf, BUFSIZ);
                    buf[n] = '\0';                   // null 종료
                    strcpy(uid, strtok(buf, "\n"));  // 개행 문자 제거
                    memset(buf, 0, BUFSIZ);

                    Send(csock, "Input Password >> ", NULL);
                    n = read(csock, buf, BUFSIZ);
                    buf[n] = '\0';                   // null 종료
                    strcpy(upw, strtok(buf, "\n"));  // 개행 문자 제거
                    memset(buf, 0, BUFSIZ);

                    Send(csock, "Input Name >> ", NULL);
                    n = read(csock, buf, BUFSIZ);
                    buf[n] = '\0';                     // null 종료
                    strcpy(uname, strtok(buf, "\n"));  // 개행 문자 제거
                    memset(buf, 0, BUFSIZ);

                    // csv 파일(세마포어 필요)로 회원가입
                    sem_wait(sem);

                    csv_fp = fopen("login.csv", "a");
                    fprintf(csv_fp, "%s, %s, %s\n", uid, upw, uname);
                    fclose(csv_fp);

                    sem_post(sem);

                    strcpy(user.id, uid);
                    strcpy(user.name, uname);

                    Send(csock, "Register Success !!\n", NULL);
                    break;
                }
                else if (num == 3) {
                    close(csock);
                    goto CLOSE;  // 해당 클라이언트 listen 종료
                }
                else {
                    Send(csock, "\033[2J\033[1;1H", NULL);
                    Send(csock, "Invalid input !!\n", NULL);
                    getchar();
                    continue;
                }
            }

            ///// todo: 채팅 그룹 설정

            // 최대 접속자 제한?
            // 동시 접속시 -> 세마포어?

            // 채팅

            do {
                char buf[BUFSIZ];

                memset(buf, 0, BUFSIZ); 

                if ((n = read(csock, buf, BUFSIZ)) <= 0) {
                    perror("read()");
                    return -1;
                }

                printf("%s[%s]: %s", user.id, user.name, buf);

                Send(csock, buf, &user);

                // 다른 client에도 write -> broadcast
                // client 접속 정보 (csock, cliaddr등) 저장하기 위한 배열 필요
                // 공유메모리 등 ipc 사용 필요

                // !list, 리스트, 귓속말, 강조 등 기능 추가
                // 파일 전송?
                
                // free(buf);
            } while (strncmp(mesg, "q", 1));
        }
        else if (pid > 0) {  // Parent Proc.
            close(csock);

            // 클라이언트들이 보내는 메시지 read 대기
        }
        else {
            perror("fork()");
        }

        close(csock);

    } while (strncmp(mesg, "q", 1));

CLOSE:;
    sem_close(sem);
    sem_unlink("login_sem");
    close(ssock);

    return 0;
}
