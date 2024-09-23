#include <wiringPi.h>
#include <stdio.h>
#include <stdbool.h>

#define SW  5   // gpio24
#define LED 1   // gpio18

// pull up 저항으로 연결됨

int switchControl() {
    int i;

    pinMode(SW, INPUT);
    pinMode(LED, OUTPUT);

    while (true) {
        if (digitalRead(SW) == LOW) {
            digitalWrite(LED, HIGH);
            delay(1000);
            digitalWrite(LED, LOW);
        }

        delay(10);
    }

    return 0;
}

int main(int argc, char** argv) {
    wiringPiSetup();
    switchControl();
    return 0;
}