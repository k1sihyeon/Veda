#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void fileRead(vector<string> &v, ifstream &fin) {
    string line;
    while (getline(fin, line)) {
        v.push_back(line);
    }
}

void search(vector<string> &v, string word) {
    for (int i = 0; i < (int)v.size(); i++) {
        int index = v[i].find(word);
        if (index != -1)
            cout << v[i] << endl;
    }
}

int main(void) {

    vector<string> wordVector;
    ifstream fin("words.txt");
    if (!fin) {
        cout << "cannot open words.txt" << endl;
        return 0;
    }

    fileRead(wordVector, fin);
    fin.close();

    cout << "words.txt 파일을 읽었습니다." << endl;

    while(true) {
        cout << "검색할 단어를 입력하세요 >> ";
        string word;
        getline(cin, word)
    }



}