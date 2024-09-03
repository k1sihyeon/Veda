#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFOFILE    "fifo"

int main(int argc, char** argv) {
    int n, fd;
    char buf[BUFSIZ];

    //FIFO 파일 열기
    if ((fd = open(FIFOFILE, O_WRONLY)) < 0) {
        perror("open()");
        return -1;
    }

    // stdin으로 입력
    while ((n = read(0, buf, sizeof(buf))) > 0)
        write(fd, buf, n);
        // 입력받은 값을 FIFO에 쓰기
    
    close(fd);

    return 0;
}