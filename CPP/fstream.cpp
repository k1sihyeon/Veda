#include <iostream>
#include <fstream>

using namespace std;

int main() {
    char name[10], dept[20];
    int sid;

    cout << "이름 >> ";
    cin >> name;

    cout << "학번(숫자로) >> ";
    cin >> sid;

    cout << "학과 >> ";
    cin >> dept;

    string dir = "c:\\Temp\\student.txt";
    ofstream fout(dir);
    if(!fout) {
        cout << "cannot open " << dir;
        return 0;
    }

    fout << name << endl;
    fout << sid << endl;
    fout << dept << endl;

    fout.close();

    ifstream fin;
    fin.open(dir);

    if (!fin) {
        cout << "cannot open file";
        return 0;
    }

    fin >> name;
    fin >> sid;
    fin >> dept;

    fin.close();

    cout << name << endl;
    cout << sid << endl;
    cout << dept << endl;

    return 0;
}