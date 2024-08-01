#include <bits/stdc++.h>
using namespace std;

template <typename T>
void print(T arr[], int n) {
    for (int i = 0; i < n; i++)
        cout << arr[i] << "\t";
    cout << endl;
}

int main(void) {

    int x[] = {1, 2, 3, 4, 5};
    double d[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    print(x, 5);
    print(d, 5);

    char c[5] = {1, 2, 3, 4, 5};
    print(c, 5);

    return 0;
}