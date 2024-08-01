#include <bits/stdc++.h>
#include <string>

using namespace std;

void swap(int *a, int *b) {
    int tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

void swap2(int *x, int *y) {
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
}

void swap3(int* x, int* y) {
    *x += *y;
    *y = *x - *y;
    *x -= *y;
}

int main(void) {

    int m = 2, n = 9;
    swap(&m, &n);
    cout << m << " " << n << "\n";

    swap2(&m, &n);
    cout << m << " " << n << "\n";

    swap3(&m, &n);
    cout << m << " " << n << "\n";
    
    return 0;
}