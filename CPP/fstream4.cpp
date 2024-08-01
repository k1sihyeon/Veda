#include <iostream>
#include <fstream>

using namespace std;

int main() {

    string dir1 = "c:\\Temp\\student.txt";
    string dir2 = "c:\\windows\\system.ini";

    fstream fout(dir1, ios::out | ios::app);
    if (!fout) {
        cout << "cannot open " << dir1;
        return 0;
    }

    fstream fin(dir2, ios::in);
    if (!fin) {
        cout << "cannot open " << dir2;
        return 0;
    }

    int c;
    while((c = fin.get()) != EOF) 
        fout.put(c);

    fin.close();
    fout.close();

    return 0;
}