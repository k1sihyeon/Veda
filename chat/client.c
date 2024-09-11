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
    char id[BUFSIZ];
    char name[BUFSIZ];
    char mesg[BUFSIZ];
} Message;

void sigfunc(int no) {
    printf("Signal from child : %d\n", no);
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
        //

        while (true) {
            Message* msgb = (Message *)malloc(sizeof(Message));
            memset(buf, 0, BUFSIZ);  // mesg 지우기
            memset(msgb, 0, sizeof(Message));
            memset(msgb->id, 0, BUFSIZ);
            memset(msgb->name, 0, BUFSIZ);
            memset(msgb->mesg, 0, BUFSIZ);

            // 서버 -> 소켓 -> mesg
            if (recv(ssock, msgb, sizeof(Message), 0) <= 0) {
                perror("recv()");
                return -1;
            }

            // mesg 출력
            printf("%s[%s]: %s", msgb->id, msgb->name, msgb->mesg);
            fflush(stdout);

            free(msgb);
        }
    }
    else if (pid > 0) {  // Parent Process - 사용자가 입력한 메시지를 서버로 전송
        char buf[BUFSIZ];

        while (true) {
            memset(buf, 0, BUFSIZ);  // mesg 지우기

            fflush(stdin);
            // 사용자 메시지 키보드 입력
            fgets(buf, BUFSIZ, stdin);
            
            // 메시지를 소켓으로 변환 후 전송
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