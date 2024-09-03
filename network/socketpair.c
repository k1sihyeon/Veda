#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <sys/socket.h>

int main(int argc, char** argv) {

    int ret, sock_fd[2];
    int status;
    char buf[] = "Hello World";
    char line[BUFSIZ];
    pid_t pid;

    // socketpair 함수가 sock_fd 배열에 fd 저장 -> fd는 0,1,2 다음 3부터 시작함
    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_fd);
    if (ret == -1) {
        perror("socketpair()");
        return -1;
    }

    printf("socket 1 : %d\n", sock_fd[0]);
    printf("socket 2 : %d\n", sock_fd[1]);

    if ((pid = fork()) < 0) {
        perror("fork()");
    }
    else if (pid == 0) {    // in Child Proc.
        write(sock_fd[0], buf, strlen(buf) + 1);
        printf("Data send : %s\n", buf);
        
        close(sock_fd[0]);
        // 자식 프로세스에서 sock_fd[0]에 write
    }
    else {                  // in Parent Proc.
        wait(&status);
        // 모든 자식 프로세스 기다림
        // waitpid는 특정 자식 프로세스만 기다림
        // status에 종료 상태의 값 저장

        read(sock_fd[1], line, BUFSIZ);
        printf("Received data : %s\n", line);

        close(sock_fd[1]);
        // 부모 프로세스에서는 자식 프로세스의 종료를 기다리고
        // sock_fd[1]의 값을 읽어서 출력

        printf("WEXITSTATUS: %d\n", WEXITSTATUS(status));
        // 다양한 매크로 통해 status 값 확인 가능
    }

    return 0;
}