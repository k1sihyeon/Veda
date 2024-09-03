#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>

int cnt = 0;
static int semid;

// 감소 연산 수행
void p() {
    struct sembuf pbuf;
    pbuf.sem_num = 0;
    pbuf.sem_op = -1;
    pbuf.sem_flg = SEM_UNDO;

    if (semop(semid, &pbuf, 1) == -1)
        perror("p : semop()");
}

// 증가 연산 수행
void v() {
    struct sembuf vbuf;
    vbuf.sem_num = 0;
    vbuf.sem_op = 1;
    vbuf.sem_flg = SEM_UNDO;

    if (semop(semid, &vbuf, 1) == -1)
        perror("v : semop()");
}

int main(int argc, char** argv) {
    union semun {
        int val;
        struct  semid_ds *buf;
        unsigned short int *array;
    } arg;

    //세마포어 채널 얻기
    if ((semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
        perror("segment()");
        return -1;
    }

    //세마포어 값을 1로
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semget()");
        return -1;
    }

    while (1) {
        if (cnt >= 8) {
            cnt--;
            p();
            printf("decrease : %d\n", cnt);
            break;
        }
        else {
            cnt++;
            v();
            printf("increase : %d\n", cnt);
            usleep(100);
        }
    }

    //세마포어 채널 삭제
    if (semctl(semid, 0, IPC_RMID, arg) == -1) {
        perror("semctl() : IPC_RMID");
        return -1;
    }

    return 0;
}