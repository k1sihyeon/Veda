#include <bits/stdc++.h>
using namespace std;

class Point {
   protected:
    int x;
    int y;

   public:
    void set(int x, int y) {
        this->x = x;
        this->y = y;
    }

    void showPoint() {
        cout << "(" << x << "," << y << ")" << endl;
    }
};

class ColorPoint : public Point {
   private:
    string color;

   public:
    void setColor(string color) {
        this->color = color;
    }
    void showColorPoint();
    bool equals(ColorPoint p);
};

void ColorPoint::showColorPoint() {
    cout << color << ":";
    showPoint();
}

bool ColorPoint::equals(ColorPoint p) {
    if (x == p.x && y == p.y && color == p.color)
        return true;
    else
        return false;
}

int main(void) {
    
    Point p;
    p.set(2, 3);
//    p.x = 5;
//    p.y = 5;
    p.showPoint();

    ColorPoint cp;
//    cp.x = 10;
//    cp.y = 10;
    cp.set(3, 4);
    cp.setColor("Red");

    cp.set(3, 4);
    cp.setColor("Blue");


    // Point p;
    // ColorPoint cp;

    // cp.set(3, 4);
    // cp.setColor("Red");

    // cp.showColorPoint();
    // cp.showPoint();

    // ///
    // ColorPoint* cpp;
    // Point* pp = &cp;
    // //up-casting

    // pp->set(2, 5);
    // pp->showPoint();

    // cpp = (ColorPoint *)pp;
    // //down-casting

    // cpp->setColor("Blue");
    // cpp->showColorPoint();

    return 0;
}