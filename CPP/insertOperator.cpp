#include <bits/stdc++.h>
using namespace std;

class Point {
   private:
    int x, y;

   public:
    Point(int x = 0, int y = 0) {
        this->x = x;
        this->y = y;
    }

    friend istream& operator>>(istream& ins, Point& a);
    friend ostream& operator<<(ostream& stream, Point a);
};

ostream& operator<<(ostream& stream, Point a) {
    stream << "(" << a.x << "," << a.y << ")";
    return stream;
}

istream& operator>>(istream& ins, Point& a) {
    cout << "x 좌표>> ";
    ins >> a.x;

    cout << "y 좌표>> ";
    ins >> a.y;

    return ins;
}

int main(void) {
    Point p(3, 4);
    cout << p << endl;

    Point q(1, 100), r(2, 200);
    cout << q << r << endl;

    Point my;
    cin >> my;
    cout << my;
}