#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

//module 2의 gpio_module 사용함

int main(int argc, char** argv) {
    char buf[BUFSIZ];
    char i;
    int fd = -1;

    memset(buf, 0, BUFSIZ);

    printf("GPIO Set : %s\n", argv[1]);

    fd = open("/dev/gpioled", O_RDWR);
    write(fd, argv[1], strlen(argv[1]));
    read(fd, buf, strlen(argv[1]));

    printf("Read Data : %s\n", buf);

    close(fd);

    return 0;
}