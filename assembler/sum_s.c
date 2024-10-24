#include <stdio.h>

extern long Sum(long* values, long length);
// .global Sum

int main(void) {
    long arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    long result = Sum(arr, 10);

    printf("%ld\n", result);

    return 0;
}
