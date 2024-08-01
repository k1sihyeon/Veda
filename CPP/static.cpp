#include <bits/stdc++.h>
using namespace std;

class Math {
   public:
    static int abs(int a) { return a > 0 ? a : -a; }
    static int max(int a, int b) { return (a > b) ? a : b; }
    static int min(int a, int b) { return (a > b) ? b : a; }
};

class Person {
   public:
    int money;
    void addMoney(int money) {
        this->money += money;
    }

    static int sharedMoney;
    static void addShared(int n) {
        sharedMoney += n;
    }
};

// static 전역 변수 선언 및 정의
int Person::sharedMoney = 10;

int main(void) {

    cout << Math::abs(-5) << "\n";
    cout << Math::max(10, 8) << "\n";
    cout << Math::min(-3, -8) << "\n";

    Person han;
    Person lee;

    han.money = 100;
    han.sharedMoney = 200;

    lee.money = 150;
    lee.addMoney(200);
    lee.addShared(200);

    cout << han.money << " " << lee.money << " " << endl;
    cout << han.sharedMoney << " " << lee.sharedMoney << " " << endl;
}