#include <iostream>
#include <fstream>

using namespace std;

int main(void) {
    ofstream fout("");
    if (!fout) {
        cout << "error";
        return 0;
    }

    string tel, name;
    cin >> tel >> name;
    fout << tel << name;
    fout.close();
}