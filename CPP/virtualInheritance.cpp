#include <bits/stdc++.h>
using namespace std;

class BaseIO {
   public:
    int mode;
};

class In : virtual public BaseIO {
   public:
    int readPos;
};

class Out : virtual public BaseIO {
   public:
    int writePos;
};

class InOut : public In, public Out {
   public:
    bool safe;
};

int main(void) {
    InOut inObj;

    inObj.readPos = 10;
    inObj.writePos = 20;
    inObj.safe = true;
    inObj.mode = 5; //mode는 하나가 됨
}