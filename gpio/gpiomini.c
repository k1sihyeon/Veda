#include <wiringPi.h>
#include <softTone.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

#define SW      5   // gpio 24
#define CDS     0   // gpio 17
#define LED     1   // gpio 18
#define SPKR    6   // gpio 25

static bool alram = false;  // 알람 설정 flag
static bool isRun = true;   // 스레드 종료 flag

// int notes[] = {
//     659, 659, 0, 659, 0, 523, 659, 0, 784, 0,
//     392, 0, 523, 392, 330, 0,
//     440, 494, 466, 440, 392, 659, 784, 880,
//     523, 392, 330, 440, 494, 466
// };

int notes[] = {
    261, 329, 392, 440, 0,         // C4, E4, G4, A4, 쉼표
    392, 329, 293, 261, 0,         // G4, E4, D4, C4, 쉼표
    261, 329, 392, 440, 0,         // C4, E4, G4, A4, 쉼표
    392, 329, 293, 261, 0,         // G4, E4, D4, C4, 쉼표
    261, 293, 329, 392, 523, 0,    // C4, D4, E4, G4, C5, 쉼표
    523, 392, 329, 293, 261, 0     // C5, G4, E4, D4, C4, 쉼표
};

void sigfunc(int signo);
int musicPlay();

void *switchFunc(void* arg) {
    while (isRun) {
        if (digitalRead(SW) == LOW) {
            alram = !alram;

            printf("alram is %s\n", alram ? "true" : "false");

            if (alram == false) {
                softToneWrite(SPKR, 0);
                softToneStop(SPKR);
            }

            delay(1000);
        }
    }
}

void *lightFunc(void* arg) {
    while (isRun) {
        if (digitalRead(CDS) == LOW) {   
            pinMode(LED, OUTPUT);  
            digitalWrite(LED, HIGH);
            delay(10);
        }

        else if (digitalRead(CDS) == HIGH) {
            if (alram) {
                musicPlay();
            }
        }
    }
}

int main() {
    pthread_t ptLight, ptSwitch;

    signal(SIGINT, sigfunc);
    wiringPiSetup();
    
    pinMode(SW, INPUT);
    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);

    pthread_create(&ptLight, NULL, lightFunc, NULL);
    pthread_create(&ptSwitch, NULL, switchFunc, NULL);

    printf("q : Quit\n");
    while (isRun) {
        if (getchar() == 'q') {
            sigfunc(0);
            break;
        }
    }

    return 0;
}

void sigfunc(int signo) {
    isRun = false;

    delay(10);

    digitalWrite(LED, LOW);
    softToneWrite(SPKR, 0);
    softToneStop(SPKR);    

    printf("goodbye!\n");

    delay(1000);
    exit(0);
}

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
