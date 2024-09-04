
// select -> 병행 처리

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_PORT 5200

int main(int argc, char** argv) {
    int ssock;
    socklen_t clen;
    int n;
    struct sockaddr_in servaddr, cliaddr;
    char mesg[BUFSIZ];

    fd_set readfd;
    int maxfd;

    int client_index, start_index;
    int client_fd[5] = {0};

    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);
    
    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind()");
        return -1;
    }

    if (listen(ssock, 8) < 0) {
        perror("listen()");
        return -1;
    }

    FD_ZERO(&readfd);   //fd_set readfd를 0으로 초기화
    maxfd = ssock;      //현재 최대 파일 디스크립터의 버호는 서버 소켓의 디스크립터
    client_index = 0;

    do {

        FD_SET(ssock, &readfd);     //서버 소켓의 읽기 감지를 위한 fd_set 설정

        //클라이언트의 시작 주소부터 마지막 주소까지 fd_set 설정
        for (start_index = 0; start_index < client_index; start_index++) {
            FD_SET(client_fd[start_index], &readfd);

            //maxfd는 클라이언트의 가장 큰 소켓 번호 저장
            if (client_fd[start_index] > maxfd)
                maxfd = client_fd[start_index];
        }
        maxfd += 1;
        // 감시할 최대 fd에 +1 필요

        // maxfd가 mask 역할

        // select 함수에서 읽기가 가능한 부분만 조사
        select(maxfd, &readfd, NULL, NULL, NULL);   //읽기가 가능할 때까지 block됨

        // 읽기 가능한 소켓이 서버 소켓인 경우
        if (FD_ISSET(ssock, &readfd)) {     
            clen = sizeof(struct sockaddr_in);

            // 클라이언트 연결 accept
            int csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
            if (csock < 0) {
                perror("accept()");
                return -1;
            }
            else {
                inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, mesg, BUFSIZ);
                printf("Client is connected : %s\n", mesg);

                // 새로 접속한 클라리언트의 소켓 번호를 fd_set에 추가
                FD_SET(csock, &readfd);
                client_fd[client_index] = csock;
                client_index += 1;
                continue;
            }

            if (client_index == 5)
                break;
        }

        // else if (FD_ISSET(client_fd[???], &readfd))
        // 읽기 가능한 소켓이 클라이언트인 경우
        for (start_index = 0; start_index < client_index; start_index++) {

            // 모든 클라이언트를 순회하며 읽기 가능한지 조사
            if (FD_ISSET(client_fd[start_index], &readfd)) {
                memset(mesg, 0, sizeof(mesg));

                // 읽기 가능한 클라이언트에서 메시지를 읽고 echo
                if ((n = read(client_fd[start_index], mesg, sizeof(mesg))) > 0) {
                    printf("Received data : %s", mesg);
                    write(client_fd[start_index], mesg, n);

                    //close(client_fd[start_index]);
                    FD_CLR(client_fd[start_index], &readfd);

                    //client_index -= 1;
                }
            }
        }

    } while (strncmp(mesg, "q", 1));

    close(ssock);

    return 0;
}