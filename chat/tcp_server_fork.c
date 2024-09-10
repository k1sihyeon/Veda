#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TCP_PORT    5100
#define MAX_CLIENT  10

static int CHILD_COUNT = 0;

// client
// 1. 사용자 지정 이름, 색상 설정
// 2. 메시지 입력, 전송
// 3. 다른 client 메시지 수신, 출력 -> fork

void sigfunc(int no) {
    printf("Signal from child(%d) : %d\n", CHILD_COUNT, no);
    CHILD_COUNT -= 1;
    
    if (CHILD_COUNT == 0)
        exit(0);
}

int main(int argc, char** argv) {
    pid_t pid;
    socklen_t clen;
    struct sockaddr_in servaddr, cliaddr;

    int n;
    int port = TCP_PORT;
    int ssock;

    char mesg[BUFSIZ];

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
        int csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
        inet_ntop(AF_INET, &cliaddr.sin_addr, ip, BUFSIZ);
        printf("Client is connected : %s\n", ip);
        CHILD_COUNT += 1;

        if ((pid = fork()) < 0) 
            perror("fork()");
        else if (pid == 0) {    // Child Proc. - 클라이언트가 연결될 때 마다
            close(ssock);

            // 최대 접속자 제한?
            // 동시 접속시 -> 세마포어?

            do {
                memset(mesg, 0, BUFSIZ);

                if ((n = read(csock, mesg, BUFSIZ)) <= 0)
                    perror("read()");
                
                mesg[n] = '\0';

                printf("Received data : %s", mesg);

                if (write(csock, mesg, n) <= 0)
                    perror("write()");
                
                // 다른 client에도 write
                // client 접속 정보 (csock, cliaddr등) 저장하기 위한 배열 필요
                // 공유메모리 등 ipc 사용 필요

                // !list, 리스트, 귓속말, 강조 등 기능 추가
                // 파일 전송?

            } while (strncmp(mesg, "q", 1));
        }

        close(csock);

    } while (strncmp(mesg, "q", 1));

    close(ssock);

    return 0;
}

