#include <bits/stdc++.h>
using namespace std;

void print(int n) {
    cout << n << " ";
}

int main(void) {

    vector<int> v = {1, 2, 3, 4, 5};

    for_each(v.begin(), v.end(), print);
    for_each(v.begin(), v.end(), [](int n){ cout << n << " "; });

    return 0;
}
