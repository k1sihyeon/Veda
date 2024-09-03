#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

sem_t* sem;
static int cnt = 7;
static bool isRun = true;

void p() {
    sem_wait(sem);
    cnt--;
}

void v() {
    sem_post(sem);
    cnt++;
}

void *pthreadV(void *arg) {
    int i;
    while (isRun) {
        usleep(150000);
        v();
        printf("increase : %d\n", cnt);
        fflush(NULL);
    }

    return NULL;
}

void *pthreadP(void *arg) {
    while (isRun) {
        p();
        printf("decrease : %d\n", cnt);
        fflush(NULL);
        if (cnt == 0)
            isRun = false;
        usleep(100000);
    }

    return NULL;
}

int main(int argc, char** argv) {

    pthread_t ptv, ptp;

    const char* name = "posix_sem";
    unsigned int value = cnt;

    sem = sem_open(name, O_CREAT, S_IRUSR | S_IWUSR, value);

    pthread_create(&ptv, NULL, pthreadV, NULL);
    pthread_create(&ptp, NULL, pthreadP, NULL);

    pthread_join(ptv, NULL);
    pthread_join(ptp, NULL);

    sem_close(sem);
    //printf("sem_destroy() : %d\n", sem_destroy(sem));

    sem_unlink(name);

    return 0;
}