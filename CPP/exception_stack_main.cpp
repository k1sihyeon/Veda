#include <iostream>
using namespace std;

#include "exception_stack.h"

int main(void) {

    MyStack intStack;

    try {
        intStack.push(100);
        intStack.push(200);

        cout << intStack.pop() << endl;
        cout << intStack.pop() << endl;
        cout << intStack.pop() << endl;
    }
    catch (const char* s) {
        cout << "예외 발생 : " << s << endl;
    }

    return 0;
}