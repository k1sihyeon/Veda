#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Histogram {
   private:
    string origin = "";
    int count = 0;
    int alpha[26];


    bool isalpha(char c) {
        if ((c >= 'a' && c <= 'z') || ((c >= 'A') && (c <= 'Z')))
            return true;
        else
            return false;
    }
    int toidx(char c) {
        if (c >= 'a' && c <= 'z')
            return c - 'a';

        else if (c >= 'A' && c <= 'Z')
            return c - 'A';
    }

   public:
    Histogram(string s) {
        memset(alpha, 0, sizeof(alpha));

        for (int i = 0; i < (int)s.length(); i++) {
            if (isalpha(s[i]))
                this->alpha[toidx(s[i])] += 1;
                count += 1;
        }

        origin += s;
        //origin += "\n";
    }
    void put(string s) {
        for (int i = 0; i < (int)s.length(); i++) {
            if (isalpha(s[i])) {
                alpha[toidx(s[i])] += 1;
                count += 1;
            }   
        }

        origin += s;
    }
    void putc(char c) {
        if (isalpha(c)) {
            alpha[toidx(c)] += 1;
            count += 1;
        }
        origin += c;            
    }
    void print() {
        cout << origin << endl;
        cout << endl;
        cout << "총 알파벳 수 " << count << endl;

        for (int i = 0; i < 26; i++) {
            cout << (char)(i + 'a') << " (" << alpha[i] << ")" << "\t:";
            cout << " ";

            for (int j = 0; j < alpha[i]; j++)
                cout << "*";
            
            cout << endl;
        }
    }

    Histogram& operator<< (char c);
    Histogram& operator<< (string s);
    Histogram& operator! (void);
};

Histogram& Histogram::operator << (char c) {
    this->putc(c);
    return *this;
}

Histogram& Histogram::operator << (string s) {
    this->put(s);
    return *this;
}

Histogram& Histogram::operator! (void) {
    this->print();
    return *this;
}

int main(void) {
    Histogram elvis("Wise men say, only fools rush in But I can't help, ");
    elvis.put("falling in love with you");
    elvis.putc('-');
    elvis.print();

    elvis << "hello";
    elvis << 'k' << 'i' << 't';
    !elvis;
}
