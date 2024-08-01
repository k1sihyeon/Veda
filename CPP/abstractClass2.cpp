#include <bits/stdc++.h>
using namespace std;

class Calculator {
   private:
    void input() {
        cout << "정수 2개를 입력하세요>> ";
        cin >> a >> b;
    }

   protected:
    int a, b;
    virtual int calc(int a, int b) = 0;

   public:
    void run() {
        input();
        cout << "계산된 값은 " << calc(a, b) << endl;
    }
};

class Adder : public Calculator {
    protected:
    int calc(int a, int b) {
        return a + b;
    }
};

class Substractor : public Calculator {
    protected:
    int calc(int a, int b) {
        return a - b;
    }
};

int main(void) {

    Adder adder;
    Substractor substractor;

    adder.run();
    substractor.run();

    return 0;
}