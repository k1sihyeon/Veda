#include <bits/stdc++.h>

using namespace std;

class Person {
   public:
    string name;
    Person (string s) {
        name = s;
    }
    string getName() {
        return this->name;
    }
};

class UpAndDownGame {
   private:
    static int ans;

   public:
    static int minE;
    static int maxE;
    static void setAns(int num) {
        ans = num;
    }

    static bool isThere(int num) {
        if (ans < num) {
            maxE = num;
            return false;
        }
        else if (ans > num) {
            minE = num;
            return false;
        }
        else {  // same
            return true;
        }
    }

    static void printBetween() {
        printf("답은 %d(와)과 %d사이에 있습니다.\n", minE, maxE);
    }

    UpAndDownGame(int a, int b) {
        minE = a;
        maxE = b;
    }
};

int UpAndDownGame::ans = 0;
int UpAndDownGame::minE = 0;
int UpAndDownGame::maxE = 99;

int main(void) {
    srand((unsigned)time(0));
    int n = rand();
    n = n % 100;
    UpAndDownGame::setAns(n);

    cout << "Up & Down 게임을 시작합니다." << endl;
    UpAndDownGame::printBetween();

    Person players[] = {Person("김인수"), Person("오은경")};
    
    int index = 0;

    while (true) {

        int num;

        cout << players[(index % 2)].getName() << ">> ";
        cin >> num;
        if (UpAndDownGame::isThere(num) == false)
            UpAndDownGame::printBetween();
        else 
            break;
        
        index += 1;

    }

    cout << players[index % 2].getName() << "(이)가 이겼습니다!!" << endl;;

    return 0;
}