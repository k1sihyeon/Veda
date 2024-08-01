#include <bits/stdc++.h>
using namespace std;

class Circle {
    private:
        int radius;
    public:
        Circle (int radius = 1) {
            this->radius = radius;
        }

        int getRadius() {
            return this->radius;
        }
};

template <typename T>
void myswap(T& a, T& b) {
    T tmp;
    tmp = a;
    a = b;
    b = tmp;
}

int main(void) {

    int a = 4, b = 5;
    myswap(a, b);
    cout << "a, b: " << a << ", " << b << endl;
    
    double c = 0.3, d = 12.5;
    myswap(c, d);
    cout << "c, d: " << c << ", " << d << endl;

    Circle donut(5), pizza(20);
    myswap(donut, pizza);
    cout << "donut, pizza: " << donut.getRadius() << ", " << pizza.getRadius() << endl;

    return 0;
}