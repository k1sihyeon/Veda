#include <bits/stdc++.h>
using namespace std;

class TV {
   private:
    int size;

   public:
    TV() {
        size = 20;
        cout << "TV Constructor without parameter" << endl;
    }

    TV(int size) {
        this->size = size;
        cout << "TV Constructor with parameter" << endl;
    }

    int getSize() {
        return size;
    }
};

class WideTV : public TV {
   private:
    bool videoIn;

   public:
    WideTV(int size, bool videoIn) : TV(size) {  // 명시적 상위 클래스 생성자 선택
        this->videoIn = videoIn;
        cout << "WideTV Constructor" << endl;
    }
    bool getVideoIn() {
        return videoIn;
    }
};

class SmartTV : public WideTV {
   private:
    string ipAddr;

   public:
    SmartTV(string ipAddr, int size) : WideTV(size, true) {
        this->ipAddr = ipAddr;
        cout << "SamrtTV Constructor" << endl;
    }

    string getIpAddr() {
        return ipAddr;
    }
};

int main(void) {
    SmartTV htv("192.168.0.1", 32);

    cout << "size = " << htv.getSize() << endl;
    cout << "videoIn = " << boolalpha << htv.getVideoIn() << endl;
    cout << "Ip = " << htv.getIpAddr() << endl;

    return 0;
}