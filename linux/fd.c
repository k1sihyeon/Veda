#include <stdio.h>
#include <unistd.h>

int main() {
    int a;
    scanf("%d", &a);            //stdin     : 0
    printf("Hello world!\n");   //stdout    : 1
    perror("Hello Error!");   //stderr    : 2

    return 0;
}
