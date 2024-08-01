#include <bits/stdc++.h>
using namespace std;

class Power {
    private:
        int kick;
        int punch;

    public:
        Power(int kick = 0, int punch = 0) {
            this->kick = kick;
            this->punch = punch;
        }
        void show();
        
        Power operator+ (Power op2);    //power + power
        Power operator+ (int op2);      //power + int   //
        friend Power operator+ (int op1, Power op2);

        Power operator== (Power op2);

        Power& operator+= (Power op2);
        Power& operator++ ();       //전위
        Power operator++(int x);    //후위

        bool operator!();

        Power& operator << (int n);
};

void Power::show() {
    cout << "kick=" << kick << ", punch=" << punch << endl;
}

Power Power::operator+ (Power op2) {
    Power temp;
    temp.kick = this->kick + op2.kick;
    temp.punch = this->punch + op2.punch;

    return temp;
}

Power Power::operator+ (int op2) {
    Power tmp;
    tmp.kick = kick + op2;
    tmp.punch = punch + op2;

    return tmp;    
}

Power operator+ (int op1, Power op2) {
    Power tmp;
    tmp.kick = op1 + op2.kick;
    tmp.punch = op1 + op2.punch;

    return tmp;
}

Power Power::operator== (Power op2) {
    if ((kick == op2.kick) && punch == op2.punch)
        return true;
    else
        return false;
}

Power& Power::operator+= (Power op2) {
    kick = kick + op2.kick;
    punch = punch + op2.punch;

    return *this;
}

Power& Power::operator++ () {
    kick++;
    punch++;

    return *this;
}

Power Power::operator++(int x) {
    Power tmp = *this;
    kick ++;
    punch ++;

    return tmp;
}

bool Power::operator!() {
    if (kick == 0 && punch == 0)
        return true;
    else    
        return false;
}

Power& Power::operator<< (int n) {
    kick += n;
    punch += n;

    return *this;
}

int main(void) {

    Power a(3, 5), b(4, 6), c;
    c = a + b;
    c = a.operator+(b);

    a.show();
    b.show();
    c.show();

    return 0;
}