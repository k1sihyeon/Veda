#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define TCP_PORT 5100
#define MAX_CLIENT  8

void sigfunc(int no) {
    printf("Signal: %d\n", no);
}

// 연결은 telnet localhost 5100

int main(int argc, char** argv) {
    pid_t pid;
    int ssock;
    socklen_t clen;
    int n;
    struct sockaddr_in servaddr, cliaddr;
    char mesg[BUFSIZ];

    // TCP (STREAM) socket 생성
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // servaddr 초기화
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TCP_PORT);

    // bind - 서버의 소켓 주소 설정
    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind()");
        return -1;
    }

    // 동시 접속하는 클라이언트를 위한 처리 대기 큐 설정
    // SYN 대기?
    if (listen(ssock, 8) < 0) { // 최대 8개 대기, 접속 x
        perror("listen()");
        return -1;
    }

    int cread, cwrite = 0;
    
    clen = sizeof(cliaddr);
    do {        // 부모 프로세스는 accept로 client 대기
        int n;
        // 클라이언트의 접속을 허용하고 클라이언트 소켓 생성
        int csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
        int readfd[MAX_CLIENT][2], writefd[MAX_CLIENT][2];

        if (pipe(readfd[++cread]) < 0) {
            perror("pipe(readfd)");
            return -1;
        }

        if (pipe(writefd[++cwrite]) < 0) {
            perror("pipe(writefd)");
            return -1;
        }

        // client ip 주소를 mesg에 변환하여 저장
        inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);

        printf("Client is connected : %s\n", mesg);

        if ((pid = fork()) < 0) {
            perror("fork()");
        }
        else if (pid == 0) {  // in child
            close(ssock);
            // 추가

            do {
                memset(mesg, 0, BUFSIZ);

                if ((n = read(csock, mesg, BUFSIZ)) <= 0)
                    perror("read()");

                mesg[n] = '\0';

                printf("Received data : %s", mesg);
                // socket에서 읽어서 출력

                if (write(csock, mesg, n) <= 0)
                    perror("write()");

            } while (strncmp(mesg, "q", 1));

            close(csock);

            exit(0);
        }
        else {
            close(csock);
            // 추가
        }

    } while (strncmp(mesg, "q", 1));

    close(ssock);

    return 0;
}