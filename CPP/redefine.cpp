#include <bits/stdc++.h>

using namespace std;

class Base {
   public:
    virtual void f() {
        cout << "Base::f() called" << endl;
    }
};

class Derived : public Base {
   public:
    virtual void f() {
        cout << "Derived::f() called" << endl;
    }
};

class GrandDerived : public Derived {
    public:
        virtual void f() {
            cout << "GrandDerived::f() called" << endl;
        }
};

int main(void) {

    GrandDerived g;
    Base *bp;
    Derived *dp;
    GrandDerived *gp;

    bp = &g;
    dp = &g;
    gp = &g;

    bp->f();
    dp->f();
    gp->f();

    // Derived d, *pDer;
    // pDer = &d;
    // pDer->f();

    // Base* pBase;
    // pBase = pDer;
    // pBase->f();
    // //동적 바인딩 실행!!

    return 0;
}