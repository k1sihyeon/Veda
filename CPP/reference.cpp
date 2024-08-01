#include <bits/stdc++.h>

using namespace std;

int main() {
    // cout << "i\tn\trefn" << endl;
    // int i = 1, n = 2;
    // int &refn = n;
    // cout << i << '\t' << n << '\t' << refn << endl;
    
    // n = 4;
    // refn++;
    // cout << i << '\t' << n << '\t' << refn << endl;

    // refn = i;   //refn과 refn이 가리키고 있는곳에 i의 값을 넣음 //-> refn이 참조하는 것을 바꾸지 않음
    // refn++;
    // cout << i << '\t' << n << '\t' << refn << endl;

    // int *p = &refn;
    // *p = 20;
    // cout << i << '\t' << n << '\t' << refn << endl;

    int n = 10;
    int& ref = n;
    

    return 0;
}