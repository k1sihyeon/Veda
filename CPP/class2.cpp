#include <bits/stdc++.h>

using namespace std;

class Person {
    char* name;
    int id;

   public:
    Person(int id, const char* name) {
        this->id = id;
        int len = strlen(name);
        this->name = new char[len + 1];
        strcpy(this->name, name);
    }

    ~Person() {
        if (name)
            delete[] name;
    }

    void changeName(const char* name) {
        if (strlen(name) > strlen(this->name))
            return;
        strcpy(this->name, name);
    }

    void show() {
        cout << id << ", " << name << endl;
    }
};

int main(void) {

    Person father(1, "Kitae");
    Person daughter(father);

    cout << "daughter created =======" << endl;

    father.show();
    daughter.show();

    daughter.changeName("Grace");
    cout << "daugher's name changed to Grace ==========" << endl;
    father.show();
    daughter.show();

    return 0;
}