#include <iostream>
#include <fstream>

using namespace std;

int main() {
    string file = "";

    ofstream fout;
    fout.open(file, ios::out | ios::binary);
    if (!fout) {
        cout << "cannot open file";
        return 0;
    }

    int n[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    double d = 3.15;

    fout.write((char *)n, sizeof(n));
    fout.write((char*)(&d), sizeof(d));
    fout.close();

    for (int i = 0; i < 10; i++)
        n[i] = 99;
    
    d = 8.15;

    ifstream fin(file, ios::in);
    if(!fin) {
        cout << "cannot open file";
        return 0;
    }

    fin.read((char *)n, sizeof(n));
    fin.read((char *)(&d), sizeof(d));

    for (int i = 0; i < 10; i++) 
        cout << n[i] << ' ';
    
    cout << endl << d << endl;
    fin.close();

    return 0;
}