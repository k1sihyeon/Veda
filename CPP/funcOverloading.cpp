#include <bits/stdc++.h>

using namespace std;

class MyVector {
   private:
    int *p;
    int size;

   public:
    MyVector(int n = 100) {
        p = new int[n];
        size = n;
    }

    ~MyVector() { delete[] p; }
};

void star(int a = 5) {
    for (int i = 0; i < a; i++)
        cout << "*";

    cout << endl;
}

void msg(int id, string text = "Hello") {
    cout << id << ' ' << text << endl;
}

int main(void) {
    MyVector *v1, *v2;
    v1 = new MyVector();
    v2 = new MyVector(1024);

    delete v1;
    delete v2;

    // star(5);
    // star(10);

    // msg(10);
    // msg(10, "Hello");

    return 0;
}