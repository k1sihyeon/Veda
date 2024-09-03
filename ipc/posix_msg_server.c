#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>

//compile: gcc ~~ -lrt

int main(int argc, char** argv) {
    mqd_t mq;
    struct mq_attr attr;             //mq 속성 정의
    const char* name = "/posix_msq"; //mq 이름
    char buf[BUFSIZ];
    int n;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFSIZ;
    attr.mq_curmsgs = 0;

    // mq 생성 및 open
    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);

    while (1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        // 전체 버퍼의 크기 만큼 받고, rtn 값으로 정확한 크기를 받음

        switch(buf[0]) {
            case 'q':
                goto END;
                break;
            
            default:
                write(1, buf, n);
                break;
        }
    }

END:;
    mq_close(mq);
    mq_unlink(name);

    return 0;
}