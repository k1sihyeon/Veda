#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFOFILE    "fifo"

int main(int argc, char** argv) {
    int n, fd;
    char buf[BUFSIZ];

    // 기존의 FIFO 파일 삭제
    unlink(FIFOFILE);

    // 새로운 FIFO 파일 생성
    if (mkfifo(FIFOFILE, 0666) < 0) {
        perror("mkfifo()");
        return -1;
    }

    // FIFO 파일 열기
    if ((fd = open(FIFOFILE, O_RDONLY)) < 0) {
        perror("open()");
        return -1;
    }

    // FIFO 파일 읽기
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        printf("%s", buf);
        // 읽은 값 stdout 출력
    }

    close(fd);

    return 0;
}