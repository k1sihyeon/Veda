#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

 int system(const char* cmd) {
     pid_t pid;
     int status;

     if ((pid = fork()) < 0) {
         status = -1;
     }
     else if (pid == 0) {
         execl("/bin/bash", "bash", "-c", cmd, (char *)0);
         _exit(127);
     }
     else {
         while (waitpid(pid, &status, 0) < 0) {
             if (errno != EINTR) {
                 status = -1;
                 break;
             }
         }
     }

     return status;
 }

int main(int argc, char** argv, char** envp) {
    while (*envp)
        printf("%s\n", *envp++);
    
    system("who");
    system("nocommand");
    system("cal");
    system("ls");

    return 0;
}
