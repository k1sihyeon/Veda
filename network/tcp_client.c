#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TCP_PORT    5100

int main(int argc, char** argv) {

    int ssock;
    struct sockaddr_in servaddr;
    char mesg[BUFSIZ];

    if (argc < 2) {
        printf("Usage : %s <IP ADRESS>\n", argv[0]);
        return -1;
    }

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
    servaddr.sin_port = htons(TCP_PORT);


    // 접속
    if (connect(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    // 사용자 메시지 키보드 입력
    fgets(mesg, BUFSIZ, stdin);

    // 메시지를 소켓으로 변환 후 전송
    if (send(ssock, mesg, BUFSIZ, MSG_DONTWAIT) <= 0) {
        perror("send()");
        return -1;
    }

    memset(mesg, 0, BUFSIZ);    // mesg 지우기

    // 서버 -> 소켓 -> mesg
    if (recv(ssock, mesg, BUFSIZ, 0) <= 0) {
        perror("recv()");
        return -1;
    }

    // mesg 출력
    printf("Received data : %s", mesg);

    // 소켓 닫기
    close(ssock);

    return 0;
}