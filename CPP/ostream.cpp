#include <iostream>

using namespace std;

int main(void) {

    cout.put('H');
    cout.put('i');
    cout.put(33);
    cout.put('\n');

    cout.put('C').put('+').put('+').put(' ');

    char str[] = "I love programming";
    cout.write(str, 6);

    return 0;
}