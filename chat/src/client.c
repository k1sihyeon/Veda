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

void sigfunc(int no) {
    //printf("Signal from child : %d\n", no);
    exit(0);
}

int main(int argc, char** argv) {
    pid_t pid;
    struct sockaddr_in servaddr;

    int ssock;
    int port = TCP_PORT;

    User user;

    /////////////////////////////////////

    if (argc < 2) {
        printf("Usage : %s <IP ADRESS>\n", argv[0]);
        return -1;
    }

    if (argc == 3) {
        port = atoi(argv[2]);
    }

    signal(SIGCHLD, sigfunc);

    // socket 생성 - TCP (STREAM)
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // 서버 주소 초기화
    memset(&servaddr, 0, sizeof(servaddr));
    // 명령어 인수의 string IP주소를 servaddr에 네트워크 형식으로 변환
    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    // 접속
    if (connect(ssock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    if ((pid = fork()) < 0) {
        perror("fork()");
        return -1;
    }
    else if (pid == 0) {  // Child Process - 서버로부터 받은 메시지를 출력
        
        char buf[BUFSIZ];
        memset(&user, 0, sizeof(User));

        while (true) {
            Msg msg;

            // 서버 -> 소켓 -> mesg
            if (recv(ssock, &msg, sizeof(Msg), 0) <= 0) {
                perror("recv()");
                return -1;
            }

            if (msg.code == DISCON_SERVER_CODE) {   // 서버 종료
                printf("=====  Disconnected from server  =====\n");
                break;
            }
            else if (msg.code == SERVER_MSG_CODE) {    // 서버가 직접 보낸 메시지
                printf("%s", msg.buf);
            }
            else if (msg.code == CLIENT_MSG_CODE) {   // 다른 클라이언트가 보낸 메시지
                if (msg.group == user.group)
                    printf("%s[%s]: %s", msg.name, msg.id, msg.buf);
            }
            else if (msg.code == ENTER_MSG_CODE) {   // 채팅방 입장
                if (msg.group == user.group)
                    printf("%s[%s] entered the chat room\n", msg.name, msg.id);
            }
            else if (msg.code == EXIT_MSG_CODE) {   // 채팅방 퇴장
                if (msg.group == user.group)
                    printf("%s[%s] left the chat room\n", msg.name, msg.id);
            }
            // else if (msg.code == LIST_MSG_CODE) {   // 사용자 리스트 - 서버 내부 처리용
            //     printf("======== client list ========\n");
            //     printf("%s", msg.buf);
            //     printf("=============================\n");
            // }
            else if (msg.code == LIST_MSG_CODE2) {   // 사용자 리스트 - 클라이언트 출력용
                if (msg.group == user.group) {
                    printf("======== client list ========\n");
                    printf("%s", msg.buf);
                    printf("=============================\n");
                }
            }
            else if (msg.code == WHISPER_MSG_CODE) {   // 귓속말
                if (strcmp(msg.destID, user.id) == 0)
                    printf("%s[%s] whispers to me : %s", msg.name, msg.id, msg.buf);
            }
            else if (msg.code == USER_INFO_CODE) {   // user 설정 코드
                strcpy(user.id, msg.id);
                strcpy(user.name, msg.name);
                user.group = msg.group;
            }
            else if (msg.code == FILE_SEND_CODE) {   // 파일 전송
                printf("send file(%s) to server\n", msg.name, msg.id, msg.buf);

                FILE* file = fopen(msg.buf, "rb");
                if (file == NULL) {
                    printf("file open error\n");
                    continue;;
                }

                Msg fmsg;
                memset(&fmsg, 0, sizeof(Msg));
                strcpy(fmsg.filename, msg.buf);
                
                fseek(file, 0, SEEK_END);
                fmsg.filesize = ftell(file);
                fseek(file, 0, SEEK_SET);

                while (fread(fmsg.buf, 1, BUFSIZ, file) > 0) {
                    fmsg.code = FILE_SEND_CODE;
                    strcpy(fmsg.id, user.id);
                    strcpy(fmsg.name, user.name);
                    fmsg.group = user.group;

                    if (send(ssock, &fmsg, sizeof(Msg), 0) <= 0) {
                        perror("send()");
                        fclose(file);
                        continue;;
                    }
                }
            }
            else if (msg.code == FILE_RECV_CODE) {   // 파일 수신

                // 파일 청크 단위 수신
                Msg fmsg;
                memset(&fmsg, 0, sizeof(Msg));
                strcpy(fmsg.filename, msg.buf);
                fmsg.filesize = 0;

                FILE* file = fopen(msg.buf, "wb");
                if (file == NULL) {
                    printf("file open error\n");
                    continue;
                }

                int received_bytes = 0;

                // 파일을 청크 단위로 수신
                do {
                    fwrite(msg.buf, 1, BUFSIZ, file);
                    received_bytes += BUFSIZ;

                    if (recv(ssock, &msg, sizeof(Msg), 0) <= 0) {
                        perror("파일 수신 중 오류 발생");
                        break;
                    }
                } while (received_bytes < msg.filesize);

                fclose(file);
                printf("%s[%s] receives a file : %s\n", msg.name, msg.id, msg.buf);
            }
            else {
                printf("Unknown code\n");
            }
            fflush(stdout);
        }
    }
    else if (pid > 0) {  // Parent Process - 사용자가 입력한 메시지를 서버로 전송
        char buf[BUFSIZ];

        while (true) {
            memset(buf, 0, BUFSIZ);  // mesg 지우기

            fflush(stdin);
            // 사용자 메시지 키보드 입력
            fgets(buf, BUFSIZ, stdin);

            // 입력한 문자열 화면에서 지우기
            printf("\033[1A\033[2K");
            
            // 메시지를 소켓으로 전송
            if (send(ssock, buf, BUFSIZ, MSG_DONTWAIT) <= 0) {
                perror("send()");
                return -1;
            }
        }
    }
    else {
        perror("fork()");
        return -1;
    }

    // 쓰기 소켓 닫음
    shutdown(ssock, SHUT_WR);

    // 소켓 닫기
    close(ssock);

    return 0;
}