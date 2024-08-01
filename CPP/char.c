#include <stdio.h>

int main() {
    
    int a = 0, b = 0, sum1, sum2;

    sum1 = a+++a+++a++;
    sum2 = ++b+ ++b+ ++b;

    printf("%d %d %d %d\n", a, b, sum1, sum2);

    return 0;
}