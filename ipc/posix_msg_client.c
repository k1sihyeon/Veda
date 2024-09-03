#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>

int main(int argc, char** argv) {
    mqd_t mq;
    const char* name = "/posix_msq";
    char buf[BUFSIZ];

    // mq open
    mq = mq_open(name, O_WRONLY);

    // mq 전송
    strcpy(buf, "Hello, World!\n");
    mq_send(mq, buf, strlen(buf), 0);
    // 버퍼의 길이 - 사이즈 x 만큼만 받음

    // mq 전송 - q
    strcpy(buf, "q");
    mq_send(mq, buf, strlen(buf), 0);

    mq_close(mq);

    return 0;
}
