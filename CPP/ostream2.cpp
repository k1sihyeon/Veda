#include <cstdio>
#include <iostream>
#include <windows.h>

using namespace std;

int main(void) {
    int n;
    char *format = "%d\n";

    printf(format, 123);
    printf("\r%f", 45.6);
    //_sleep(5000);
    printf("\r%s%n\n", "Hello World", &n);
    printf("Length : %d\n", n);

    for (int i = 0; i <= 100; i++) {
        printf("\r%3d%%", i);
        fflush(stdout);
        Sleep(500);
    }

    
}