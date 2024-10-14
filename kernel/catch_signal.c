// gpiosignal_module : module6 필요
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

//static int fd = -1;

void signal_handler(int signum) {
    printf("Signal is Catched!!!\n");

    if (signum == SIGIO) {
        printf("SIGIO\r\n");

        // char buf[BUFSIZ];
        // sprintf(buf, "%d:%d", 0, getpid());
        // write(fd, buf, strlen(buf));
        // close(fd);

        exit(1);
    }
}

int main(int argc, char** argv) {
    char buf[BUFSIZ];
    char i = 0;
    int fd = -1;
    memset(buf, 0, BUFSIZ);

    signal(SIGIO, signal_handler);

    if (argc < 2) {
        printf("Usage : %s [0/1]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("GPIO Set : %s\n", argv[1]);
    fd = open("/dev/gpioled", O_RDWR);
    sprintf(buf, "%s:%d", argv[1], getpid());

    write(fd, buf, strlen(buf));

    if (read(fd, buf, strlen(buf)) != 0)
        printf("Success : read()\n");

    printf("Read Data : %s\n", buf);

    printf("My PID is %d.\n", getpid());

    while(1);   // infinite loop

    close(fd);

    return 0;
}