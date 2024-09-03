#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define UDP_PORT    5100

int main(int argc, char** argv) {

    int sockfd, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char mesg[1000];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // UDP(DGRAM) socket 생성

    memset(&servaddr, 0, sizeof(servaddr));         //servaddr 초기화
    servaddr.sin_family = AF_INET;                  //
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //INADDR_ANY -> 서버 자신의 주소가 동적으로 바뀌더라도 사용 가능
    servaddr.sin_port = htons(UDP_PORT);            //

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));   //

    do {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, mesg, 1000, 0, (struct sockaddr *)&cliaddr, &len);
        sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

        mesg[n] = '\0';
        printf("Received data : %s\n", mesg);
    } while (strncmp(mesg, "q", 1));
    // 클라이언트로 부터 메시지를 받아서 다시 전송 및 출력

    close(sockfd);

    return 0;
}