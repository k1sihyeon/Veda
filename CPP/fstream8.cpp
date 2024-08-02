#include <iostream>
#include <fstream>

using namespace std;

long getFileSize(ifstream& fin) {
    fin.seekg(0, ios::end);
    long length = fin.tellg();

    return length;
}

int main(void) {
    string file = "";

    ifstream fin(file);
    if(!fin) {
        cout << "cannot open " << file << endl;
        return 0;
    }

    cout << file << "의 크기는 " << getFileSize(fin);
    fin.close();

    return 0;
}