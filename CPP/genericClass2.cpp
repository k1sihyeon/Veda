#include <bits/stdc++.h>
using namespace std;

template <typename T1, typename T2>
class GClass {
    private:
        T1 data1;
        T2 data2;
    public:
        GClass();
        void set(T1 a, T2 b);
        void get(T1 &a, T2 &b);
};

template <typename T1, typename T2>
GClass<T1, T2>::GClass() {
    data1 = 0;
    data2 = 0;
}

template <typename T1, typename T2>
void GClass<T1, T2>::set(T1 a, T2 b) {
    data1 = a;
    data2 = b;
}

template <typename T1, typename T2>
void GClass<T1, T2>::get(T1 &a, T2 &b) {
    a = data1;
    b = data2;
}

int main(void) {

    int a;
    double b;
    GClass<int, double> x;
    x.set(2, 0.5);
    x.get(a, b);
    cout << "a, b = " << a << ", " << b << endl;

    char c;
    float f;
    GClass<char, float> y;
    y.set('m', 12.5);
    y.get(c, f);
    cout << "c, f = " << c << ", " << f << endl;

    return 0;
}