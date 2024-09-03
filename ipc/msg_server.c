#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>

#define MSQKEY  51234

// MQ에 넘겨줄 사용자 정의 타입 선언 
struct msgbuf {
    long mtype;
    char mtext[BUFSIZ];
};

int main(int argc, char** argv) {
    key_t key;
    int n, msqid;
    struct msgbuf mb;

    key = MSQKEY;

    // MQ의 채널 생성
    if ((msqid = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("msgget()");
        return -1;
    }

    // MQ에서 data 가져오기
    while ((n = msgrcv(msqid, &mb, sizeof(mb), 0, 0)) > 0) {
        switch (mb.mtype) {
            case 1:
                write(1, mb.mtext, n);  //type이 1 -> stdout 출력
                break;
            
            case 2:     //type: 2 -> mq channel 삭제
                if (msgctl(msqid, IPC_RMID, (struct msqid_ds *) 0) < 0) {
                    perror("msgctl()"); // channel 삭제 실패 시
                    return -1;
                }
                break;
        }
    }

    return 0;
}