#include <bits/stdc++.h>
using namespace std;

int main(void) {
    vector<string> sv;
    string name;

    cout << "input 5 names" << endl;
    for (int i = 0; i < 5; i++) {
        cout << i + 1 << ">> ";
        getline(cin, name);
        sv.push_back(name);
    }
    
    name = sv.at(0);
    for (int i = 1; i < (int)sv.size(); i++) {
        if (name < sv[i])
            name = sv[i];
    }

    cout << "the last name ordered by dic is " << name << endl;

    return 0;
}