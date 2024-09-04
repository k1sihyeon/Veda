#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT_NUM    5100

int main(int argc, char** argv) {

    pid_t pid;
    static int ssock;
    int port, status;
    struct sockaddr_in servaddr;

    if (argc < 3) {
        printf("Usage : %s <IP ADDRESS> <PORT>\n", argv[0]);
        return -1;
    }

    port = (argc == 3) ? atoi(argv[2]) : PORT_NUM;

    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (connect(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    if ((pid = fork()) < 0) {
        perror("fork()");
        return -1;
    }
    else if (pid == 0) {    //in Child Proc.

        //shutdown(ssock, SHUT_RD);

        char mesg[BUFSIZ];

        do {
            // memset(mesg, 0, BUFSIZ);
            // fflush(stdin);
            fgets(mesg, BUFSIZ, stdin);

            if (send(ssock, mesg, BUFSIZ, MSG_DONTWAIT) <= 0) {
                perror("send()");
                return -1;
            }
        } while (strncmp(mesg, "q", 1));

        printf("Child Process Done!\n");

    }
    else if (pid > 0) {     //in Parent Proc.

        //shutdown(ssock, SHUT_WR);

        char mesg[BUFSIZ];

        do {
            memset(mesg, 0, BUFSIZ);

            if (recv(ssock, mesg, BUFSIZ, 0) <= 0) {
                perror("recv()");
                return -1;
            }

            printf("[P] Received from server : %s", mesg);
        //} while (1);
        } while (waitpid(pid, &status, 0) < 0);
    }
    else {
        perror("fork()");
        return -1;
    }

    close(ssock);

    if (pid == 0)
        printf("Child Process Done!\n");
    else if (pid > 0)
        printf("Parent Process Done!\n");

    return 0;
}