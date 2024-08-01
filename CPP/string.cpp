#include <bits/stdc++.h>
#include <string>

using namespace std;

int main(void) {

    string s;
    cout << "input >> ";
    getline(cin, s, '&');
    cin.ignore();

    string f, r;
    cout << "\n" << "find: ";
    getline(cin, f, '\n');

    cout << "replace: ";
    getline(cin, s, '\n');

    int startIndex = 0;
    while(true) {
        int fIndex = s.find(f, startIndex);

        if (fIndex == -1)
            break;
        
        s.replace(fIndex, f.length(), r);
        startIndex = fIndex + r.length();
    }

    cout << s << endl;
    
    return 0;
}