#include <iostream>
using namespace std;

int main(void) {
    cout << 30 << endl;

    cout.unsetf(ios::dec);
    cout.setf(ios::hex);
    cout << 30 << endl;

    cout.setf(ios::showbase);
    cout << 30 << endl;

    cout.setf(ios::uppercase);
    cout << 30 << endl;

    cout.setf(ios::dec | ios::showpoint);

    cout << 23.5 << endl;

    cout.setf(ios::scientific);
    cout << 23.5 << endl;

    cout.setf(ios::showpos);
    cout << 23.5;

    return 0;
}