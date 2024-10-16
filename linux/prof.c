// gcc -pg -o test prof.c -lm
// ./test 실행
// gprof test gmon.out

#include <stdio.h>
#include <math.h>

void function_call(int n) {
    printf("value : %f\n", sin(n));
}

int main(void) {
    for (int i = 0; i < 10000; i++)
        function_call(i);

    return 0;
}