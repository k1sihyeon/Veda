#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#define SW      5   // gpio24
#define CDS     0   // gpio17
#define LED     1   // gpio18

// cds는 pull down으로 연결됨

void sigfunc(int signo) {
    digitalWrite(LED, LOW);
    delay(10);
    
    exit(0);
}

int cdsControl() {
    int i;

    pinMode(SW, INPUT);
    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);

    while (true) {
        if (digitalRead(CDS) == HIGH) {
            digitalWrite(LED, HIGH);
            delay(1000);
            digitalWrite(LED, LOW);
        }
    }

    return 0;
}

int main() {
    signal(SIGINT, sigfunc);
    wiringPiSetup();
    cdsControl();
    return 0;
}