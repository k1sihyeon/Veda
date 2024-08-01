#include <bits/stdc++.h>
using namespace std;

template <typename T1, typename T2>
void mcopy(T1 src[], T2 dest[], int n) {
    for (int i = 0; i < n; i++)
        dest[i] = (T2)src[i];
}

int main(void) {
    int x[] = {1, 2, 3, 4, 5};
    double d[5];
    char c[5] = {'H', 'e', 'l', 'l', 'o'};
    char e[5];

    mcopy(x, d, 5);
    mcopy(c, e, 5);

    for (int i = 0; i < 5; i++) cout << d[i] << " ";
    cout << endl;

    for (int i = 0; i < 5; i++) cout << e[i] << " ";
    cout << endl;

    return 0;
}
