#include <stdio.h>
#include <unistd.h>

static int gVar = 1;
char str[] = "PID";

int main(int argc, char** argv) {
    int var;
    __pid_t pid;
    var = 88;

    if ((pid = vfork()) < 0) {   //not created child
        perror("[ERROR] : vfork()");
    }
    else if (pid == 0) {    //in child
        gVar += 1;
        var += 1;
        printf("Parent %s in Child Process(%d) : %d\n", str, getpid(), getppid());
        printf("pid = %d, Global Var. = %d, var = %d\n", getpid(), gVar, var);
        _exit(0);
    }
    else {
        printf("Child %s in Parent Process(%d) : %d\n", str, getpid(), pid);
    }

    printf("pid = %d, Global Var. = %d, var = %d\n", getpid(), gVar, var);

    return 0;
}