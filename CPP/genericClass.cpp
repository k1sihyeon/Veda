#include <bits/stdc++.h>
using namespace std;

/////////////////////////////////////////
// MyStack start;

template <typename T>
class MyStack {
    private:
        int tos;
        T data[100];
    
    public:
        MyStack();
        void push(T element);
        T pop();
};

template <typename T>
MyStack<T>::MyStack() {
    tos = -1;
}

template <typename T>
void MyStack<T>::push(T element) {
    if (tos == 99) {
        cout << "stack full";
        return;
    }
    tos += 1;
    data[tos] = element;
}

template <typename T>
T MyStack<T>::pop() {
    T retData;

    if (tos == 1) {
        cout << "stack empty";
        return 0;
    }

    retData = data[tos--];
    return retData;
}

// MyStack end;
//////////////////////////////////

int main(void) {

    MyStack<int> i;
    i.push(3);
    cout << i.pop() << endl;

    MyStack<double> d;
    d.push(3.5);
    cout << d.pop() << endl;

    MyStack<char> *p = new MyStack<char>();
    p->push('a');
    cout << p->pop() << endl;

    delete p;

    return 0;
}