#include <iostream>
#include <fstream>

using namespace std;

int main(void) {

    string src = "";
    string dest = "";

    ifstream fsrc(src, ios::in | ios::binary);
    if(!fsrc) {
        cout << "cannot open" << src << endl;
        return 0;
    }

    ofstream fdest(dest, ios::out | ios::binary);
    if(!fdest) {
        cout << "cannot open" << dest << endl;
        return 0;
    }

    int c;
    while((c = fsrc.get()) != EOF) {
        fdest.put(c);
    }

    cout << src << " to " << dest << " :done!" << endl;

    fsrc.close();
    fdest.close();

    return 0;
}