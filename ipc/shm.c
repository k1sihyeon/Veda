#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHM_KEY 0x12345

int main(int argc, char** argv) {
    int i, pid, shmid;
    int* val;
    void* shmmem = (void *) 0;
    // void pointer -> 어떠한 포인터 변수도 받을 수 있음 (casting 필요)

    if ((pid = fork()) == 0) {  //in Child Proc.
        // 공유 메모리 get
        shmid = shmget((key_t)SHM_KEY, sizeof(int), 0);
        if (shmid == -1) {
            perror("shmget()");
            return -1;
        }

        // 공유메모리를 proc 메모리에 attach
        shmmem = shmat(shmid, (void *)0, 0666 | IPC_CREAT);
        if (shmmem == (void *) -1) {
            perror("shmat()");
            return -1;
        }

        // 공유 메모리를 가져와서
        val = (int *) shmmem;
        *val = 1;   // 값 초기화

        // 3번 1 증가
        for (i = 0; i < 3; i++) {
            *val += 1;
            printf("Child(%d) : %d \n", i, *val);
            sleep(1);
        }

    }
    else if (pid > 0) { // in Parent Proc.
        // 공유 메모리 get
        shmid = shmget((key_t) SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget()");
            return -1;
        }

        // 공유 메모리 attach
        shmmem = shmat(shmid, (void *)0, 0);
        if (shmmem == (void *)-1) {
            perror("shmat()");
            return -1;
        }

        // 공유메모리 값 가져와서
        val = (int *)shmmem;
        for (i = 0; i < 3; i++) {
            sleep(1);
            printf("Parent(%d) : %d\n", i, *val);
            // 출력하기
        }
    }

    shmctl(shmid, IPC_RMID, 0);

    return 0;
}