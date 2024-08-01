#include <bits/stdc++.h>

using namespace std;

ostream& fivestar(ostream& outs) {
    return outs << "*****";
}

ostream& rightarrow(ostream& outs) {
    return outs << "---->";
}

ostream& beep (ostream& outs) {
    return outs << '\a';
}

istream& question(istream& ins) {
    cout << "input >>";
    return ins;
}

int main(void) {
    
    cout << "벨이 울립니다." << beep << endl;
    cout << "C" << rightarrow << "C++" << rightarrow << "Java" << endl;
    cout << "Visual" << fivestar << "C++" << endl;

    string answer;
    cin >> question >> answer;
    cout << "answer is " << answer << endl;

    return 0;
}