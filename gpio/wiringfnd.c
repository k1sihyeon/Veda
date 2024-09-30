#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define PINS    4

int fndControl(int num) {
    int i;
    // int gpiopins[PINS] = {13, 16, 12, 10, 8, 3, 5};
    //                     // a, b, c, d, e, f, g

    // int number[10][PINS] = {
    //     {0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 1, 1, 1, 1},
    //     {0, 0, 1, 0, 0, 1, 0},
    //     {0, 0, 0, 0, 1, 1, 0},
    //     {1, 0, 0, 1, 1, 0, 0},
    //     {0, 1, 0, 0, 1, 0, 0},
    //     {0, 1, 0, 0, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 1, 1},
    //     {0, 0, 0, 0, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 0, 0}
    // };

    int gpiopins[PINS] = {4, 1, 16, 15};

    int number[10][PINS] = {
        {0, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 1, 0, 0},
        {0, 1, 0, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 1},
        {1, 0, 0, 0},
        {1, 0, 0, 1}
    };

    for (i = 0; i < PINS; i++) {
        pinMode(gpiopins[i], OUTPUT);
    }

    for (i = 0; i < PINS; i++) {
        digitalWrite(gpiopins[i], number[num][i] ? HIGH : LOW);
    }

    //getchar();

    delay(500);

    for (i = 0; i < PINS; i++) {
        digitalWrite(gpiopins[i], HIGH);
    }

    return 0;
}

int main(int argc, char** argv) {
    int no;

    if (argc < 2) {
        printf("Usage : %s NO\n", argv[0]);
        return -1;
    }

    no = atoi(argv[1]);
    wiringPiSetup();

    for (int i = 0; i <= no; i++) {
        fndControl(i);
    }
    
    return 0;
}