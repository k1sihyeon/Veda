#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TCP_PORT 5100

int main(int argc, char** argv) {
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
    if (listen(ssock, 8) < 0) { // 최대 8개 대기
        perror("listen()");
        return -1;
    }
    
    clen = sizeof(cliaddr);
    do {
        int n;
        // 클라이언트의 접속을 허용하고 클라이언트 소켓 생성
        // SYN + ACK?
        int csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);

        // 명령어 인수의 ip주소 변경
        inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
        printf("Client is connected : %s\n", mesg);

        if ((n = read(csock, mesg, BUFSIZ)) <= 0)
            perror("read()");

        mesg[n] = '\0';
        
        printf("Received data : %s", mesg);
        // socket에서 읽어서 출력

        if (write(csock, mesg, n) <= 0)
            perror("write()");
        
        close(csock);
    } while(strncmp(mesg, "q", 1));

    close(ssock);

    return 0;
}