#include <wiringPi.h>
#include <softTone.h>
#include <stdlib.h>
#include <signal.h>

#define SPKR    6
#define TOTAL   32

void sigfunc(int signo) {
    softToneWrite(SPKR, 0);
    softToneStop(SPKR);
    delay(10);
    exit(0);
}

// int notes[] = { 	/* 학교종을 연주하기 위한 계이름 */
// 	391, 391, 440, 440, 391, 391, 329.63, 329.63, \
// 	391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0, \
// 	391, 391, 440, 440, 391, 391, 329.63, 329.63, \
// 	391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
// };

int notes[] = {
    659, 659, 0, 659, 0, 523, 659, 0, 784, 0,
    392, 0, 523, 392, 330, 0,
    440, 494, 466, 440, 392, 659, 784, 880,
    523, 392, 330, 440, 494, 466
};

int musicPlay() {
    int i;

    softToneCreate(SPKR);

    int length = sizeof(notes) / sizeof(notes[0]);

    for (i = 0; i < length; i++) {
        softToneWrite(SPKR, notes[i]);
        delay(280);
    }

    softToneWrite(SPKR, 0);
    softToneStop(SPKR);
    delay(10);

    return 0;
}

int main() {
    signal(SIGINT, sigfunc);
    wiringPiSetup();
    musicPlay();
    return 0;
}