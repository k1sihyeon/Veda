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
#include <sys/wait.h>

#define TCP_PORT    5100
#define MAX_CLIENT  10

#define READ_FD     0
#define WRITE_FD    1

static int clientSock[MAX_CLIENT];
static int pipes[MAX_CLIENT][2];
static int child_pid[MAX_CLIENT];
static int client_count = 0;
static int my_idx = 0;

// client
// 1. 사용자 지정 이름, 색상 설정
// 2. 메시지 입력, 전송
// 3. 다른 client 메시지 수신, 출력 -> fork

typedef struct user {
    char id[BUFSIZ];
    char name[BUFSIZ];
} __attribute__((__packed__)) User;

typedef struct message {
    int code;
    char id[BUFSIZ];
    char name[BUFSIZ];
    char mesg[BUFSIZ];
} __attribute__((__packed__)) Message;

typedef struct pipem {
    Message msg;
    int idx;
    int pid;
    int code;   // 0: from child to parent, 1: from parent to child
} __attribute__((__packed__)) PipeM;

// 파이프로 통신 시에 PipeM으로 변환!!!!!!!!!!!!!!!!!!!!!!!!!

void sigchild(int signo) {
    printf("Signal from child to remove client: %d\n", signo);

    client_count--;

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (child_pid[i] == signo) {
            clientSock[i] = 0;
            break;
        }
    }

    printf("Client disconnected. Active clients: %d\n", client_count);
}

// 자식 프로세스에서 호출 -> 부모 프로세스에서 실행 - client 수신 시그널
void siguser1(int signo) {

    printf("Signal(SIGUSR1) from child : %d\n", signo);
    fflush(stdout);

    PipeM *pm = (PipeM *)malloc(sizeof(PipeM));
    memset(pm, 0, sizeof(PipeM));

    // broadcast
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (child_pid[i] < 0)
            continue;
        
        if (read(pipes[i][READ_FD], pm, sizeof(PipeM)) > 0) {
            if (pm->code == 0) {
                for (int j = 0; j < MAX_CLIENT; j++) {
                    printf("[SIGUSR1] from pipe(%d) to pipe(%d), getpid(%d) : %s\n", i, j, getpid(), pm->msg.mesg);

                    pm->code = 1;
                    write(pipes[j][WRITE_FD], pm, sizeof(PipeM));
                    kill(child_pid[j], SIGUSR2);
                }
            }
            else if (pm->code == 1) {   
                write(pipes[i][WRITE_FD], pm, sizeof(PipeM));
                //kill(child_pid[i], SIGUSR2);
            }
        }
    }

    return;
}

// 부모 프로세스에서 호출 -> 자식 프로세스에서 실행 - client socket으로 전송 시그널
void siguser2(int signo) {
    printf("Signal(SIGUSR2) from parent(SIGUSR1) : %d\n", signo);
    fflush(stdout);

    if (my_idx < 0) {
        printf("[SIGUSR2] idx(%d) not found\n", my_idx);
        return;
    }

    PipeM *pm = (PipeM *)malloc(sizeof(PipeM));
    memset(pm, 0, sizeof(PipeM));

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (read(pipes[i][READ_FD], pm, sizeof(PipeM)) > 0) {
            if ((pm->code == 1)) {
                printf("[SIGUSR2] from pipe(%d) to csock(%d), getpid(%d) : %s\n", i, clientSock[i], getpid(), pm->msg.mesg);
                if (write(clientSock[i], &(pm->msg), sizeof(Message)) < 0) {
                    perror("[sigusr2] write() to client");
                    exit(0);
                }
            }
            else if (pm->code == 0) {
                write(pipes[i][WRITE_FD], pm, sizeof(PipeM));
                //kill(getppid() SIGUSR1);
            }
            else {
                printf("[SIGUSR2] pipe(%d): invalid code(%d)\n", i, pm->code);
            }
        }
    }

    free(pm);

    return;
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

    //size_t n = send(sockfd, msg, sizeof(Message), MSG_DONTWAIT);
    size_t n = write(sockfd, msg, sizeof(Message));
    usleep(100);
    free(msg);

    if (n <= 0) {
        perror("Send()");
        exit(0);
    }
}

int main(int argc, char **argv) {
    pid_t pid;
    socklen_t clen;
    struct sockaddr_in servaddr, cliaddr;

    FILE *csv_fp;
    sem_t *sem;

    int port = TCP_PORT;
    int ssock;

    char mesg[BUFSIZ];

    //////////////////////////////

    memset(clientSock, 0, sizeof(clientSock));

    sem = sem_open("login_sem", O_CREAT, 0644, 1);

    if (argc == 2) {
        port = atoi(argv[1]);
    }

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
        client_count += 1;
        
        // client socket 저장
        int idx;
        for (idx = 0; idx < MAX_CLIENT; idx++) {
            if (clientSock[idx] == 0) {
                clientSock[idx] = csock;
                break;
            }
        }

        // pipe 생성
        if (pipe(pipes[idx]) < 0) {
            perror("pipe()");
            return -1;
        }

        // pipe non-blocking 설정 - 일단 에러 처리 안함
        int flags = fcntl(pipes[idx][READ_FD], F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(pipes[idx][READ_FD], F_SETFL, flags);

        flags = fcntl(pipes[idx][WRITE_FD], F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(pipes[idx][WRITE_FD], F_SETFL, flags);
        

        if ((pid = fork()) < 0)  ///////////////////////  fork  //////////////////////////////
            perror("fork()");
        else if (pid == 0) {  // Child Proc. - 클라이언트가 연결될 때 마다 
            close(ssock);
            //close(pipes[idx][READ_FD]);

            signal(SIGUSR2, siguser2);

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
                    //char buf[BUFSIZ];
                    memset(buf, 0, BUFSIZ);

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
                    continue;
                }
            }

            ///// todo: 채팅 그룹 설정

            // 최대 접속자 제한?


            char buf[BUFSIZ];
            // 채팅
            do {
                memset(buf, 0, BUFSIZ); 

                if ((n = read(csock, buf, BUFSIZ)) <= 0) {
                    perror("read()");
                    return -1;
                }

                printf("%s[%s]: %s", user.id, user.name, buf);

                // broadcast : pipe에 쓰기
                Message *msg = (Message *)malloc(sizeof(Message));
                memset(msg, 0, sizeof(Message));
                msg->code = 0;
                strcpy(msg->id, user.id);
                strcpy(msg->name, user.name);
                strcpy(msg->mesg, buf);

                PipeM *pm = (PipeM *)malloc(sizeof(PipeM));
                memset(pm, 0, sizeof(PipeM));
                strcpy(pm->msg.id, msg->id);
                strcpy(pm->msg.name, msg->name);
                strcpy(pm->msg.mesg, msg->mesg);
                pm->msg.code = msg->code;
                pm->idx = idx;
                pm->pid = getpid();
                pm->code = 0;

                my_idx = idx;
                write(pipes[idx][WRITE_FD], pm, sizeof(PipeM));
                kill(getppid(), SIGUSR1);

                // echo 
                Send(csock, buf, &user);

                fflush(stdout);

                // 다른 client에도 write -> broadcast
                // client 접속 정보 (csock, cliaddr등) 저장하기 위한 배열 필요

                // !list, 리스트, 귓속말, 강조 등 기능 추가
                // 파일 전송?
                
                // free(buf);
            } while (strncmp(buf, "q", 1));

            kill(getppid(), SIGCHLD);
        }
        else if (pid > 0) {  //////////////////////  Parent Proc.  ////////////////////////////// 
            //close(csock);

            // child proc. signal 처리 설정
            signal(SIGUSR1, siguser1);
            signal(SIGCHLD, sigchild);

            // close(pipes[idx][WRITE_FD]);
            child_pid[idx] = pid;
        }
        else {
            perror("fork()");
        }                   //////////////// fork end //////////////////////

        //close(csock);

    } while (strncmp(mesg, "q", 1));

CLOSE:;
    sem_close(sem);
    sem_unlink("login_sem");
    close(ssock);

    return 0;
}
