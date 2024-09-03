#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    pid_t pid;
    int pfd[2];             //pfd[0] : read, pfd[1] : write
    char line[BUFSIZ];
    int status;

    if (pipe(pfd) < 0) {
        perror("pipe()");
        return -1;
    }

    if ((pid = fork()) < 0) {
        perror("fork()");
        return -1;
    }
    else if (pid == 0) {    //in Child Proc.
        close(pfd[0]);  // read pipe 닫기
        
        // stdout으로 출력됨
        write(1, "I'm in Child Process\n", 22);
        //execl("/bin/date", "date", 0);    //exec이 메모리 영역을 침범?

        // 1(stdout)을 pfd[1](pipe write)로 변경
        dup2(pfd[1], 1);   
        execl("/bin/date", "date", 0);

        close(pfd[1]);
    }
    else {                  //in Parent Proc.
        close(pfd[1]);  // write pipe 닫기

        if ((read(pfd[0], line, BUFSIZ)) < 0) {
            perror("read()");
            return -1;
        }
        printf("Parent: %s\n", line);

        close(pfd[0]);
        waitpid(pid, &status, 0);
    }

    return 0;
}
