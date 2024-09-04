#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT    5200

int main(int argc, char** argv) {
    int sockfd;
    fd_set readfds;
    char mesg[BUFSIZ];

    struct sockaddr_in servaddr;

    if (argc < 2) {
        printf("Usage : %s <IP ADDRESS>\n", argv[0]);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr);
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    do {
        // fd_set/zero 가 반복문 안에 있어야 하는 이유?
        // fd_set은 비트 마스크, select 이후 값이 변함

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sockfd, &readfds);

        // 읽을 수 있을때 까지 기다림
        select(sockfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &readfds)) {
            int size = read(0, mesg, BUFSIZ);
            write(sockfd, mesg, size);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            int size = read(sockfd, mesg, BUFSIZ);
            if (size == 0) break;
            write(1, mesg, size);
        }

    } while (strncmp("q", mesg, 1));

    close(sockfd);

    return 0;
}