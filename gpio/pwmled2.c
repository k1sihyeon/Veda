#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static int nGPIO;

void sigfunc(int signo) {
    softPwmWrite(nGPIO, 0);
    delay(10);
    exit(0);
}

void softPwmControl(int gpio) {
    pinMode(gpio, OUTPUT);
    softPwmCreate(gpio, 100, 100);

    while (1) {
        for (int i = 100; i >= 0; i--) {
            softPwmWrite(gpio, i);
            delay(10);
        }

        for (int i = 0; i <= 100; i++) {
            softPwmWrite(gpio, i);
            delay(10);
        }
    }
}

int main(int argc, char **argv) {
    int gno;
    if (argc < 2) {
        printf("error\n");
        return -1;
    }

    signal(SIGINT, sigfunc);

    gno = atoi(argv[1]);
    nGPIO = gno;

    wiringPiSetup();
    softPwmControl(gno);
    while (1)
        ;

    return 0;
}