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

typedef struct message {
    int code;
    char id[20];
    char name[50];
    char buf[BUFSIZ];
} __attribute__((__packed__)) Msg;

void sigfunc(int no) {
    //printf("Signal from child : %d\n", no);
    exit(0);
}

int main(int argc, char** argv) {
    pid_t pid;
    struct sockaddr_in servaddr;

    int ssock;
    int port = TCP_PORT;

    char mesg[BUFSIZ];

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

        while (true) {
            Msg msg;

            // 서버 -> 소켓 -> mesg
            if (recv(ssock, &msg, sizeof(Msg), 0) <= 0) {
                perror("recv()");
                return -1;
            }

            if (msg.code == -1) {   // 서버 종료
                printf("=====  Disconnected from server  =====\n");
                break;
            }
            else if (msg.code == 0) {    // 서버가 직접 보낸 메시지
                printf("%s", msg.buf);
            }
            else if (msg.code == 1) {   // 다른 클라이언트가 보낸 메시지
                printf("%s[%s]: %s", msg.name, msg.id, msg.buf);
            }
            else if (msg.code == 2) {   // 채팅방 입장
                printf("%s[%s] entered the chat room\n", msg.name, msg.id);
            }
            else if (msg.code == 3) {   // 채팅방 퇴장
                printf("%s[%s] left the chat room\n", msg.name, msg.id);
            }
            // else if (msg.code == 4) {   // 사용자 리스트 - 서버 내부 처리용
            //     printf("======== client list ========\n");
            //     printf("%s", msg.buf);
            //     printf("=============================\n");
            // }
            else if (msg.code == 5) {   // 사용자 리스트 - 클라이언트 출력용
                printf("======== client list ========\n");
                printf("%s", msg.buf);
                printf("=============================\n");
            }
            else if (msg.code == 6) {   // 귓속말
                printf("%s[%s] whispers to me : %s", msg.name, msg.id, msg.buf);
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