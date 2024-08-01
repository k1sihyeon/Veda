#include <iostream>

using namespace std;

class Rectangle {
   public:
    int width;
    int height;
    int getArea();
    bool isSquare();

    Rectangle();
    Rectangle(int a);
    Rectangle(int a, int b);
};

Rectangle::Rectangle() : Rectangle(1) {}

Rectangle::Rectangle(int a) : Rectangle(a, a) {}

Rectangle::Rectangle(int a, int b) {
    width = a;
    height = b;
}

int Rectangle::getArea() {
    return width * height;
}

bool Rectangle::isSquare() {
    return width == height;
}

////////////////////////////////////////////////////////////////////

class Circle {
   public:
    int radius;

    double getArea();
    void setRadius(int r);

    Circle();
    Circle(int r);

    ~Circle();
};

// Circle::Circle() {
//     radius = 1;
//     cout << "Circle created; radius = " << radius << endl;
// }

//위임 생성자 //delegating constructor
Circle::Circle() : Circle(1) { }

Circle::Circle(int r) {
    radius = r;
    cout << "Circle created; radius = " << radius << endl;
}

Circle::~Circle() {
    cout << "Circle Deleted !" << endl;
    //delete this;
}

double Circle::getArea() {
    return radius * radius * 3.14;
}

void Circle::setRadius(int r) {
    radius = r;
}


int main(int argc, char** argv, char** environ) {

    //Rectangle r1;
    // Rectangle r2(3, 5);
    // Rectangle r3(3);

    // if(r1.isSquare()) cout << "r1 is Square" << endl;
    // if(r2.isSquare()) cout << "r2 is Square" << endl;
    // if(r3.isSquare()) cout << "r3 is Square" << endl;


    Circle donut;
    // //donut.radius = 1;
    // double area = donut.getArea();
    // cout << "Donut area is " << area << endl;

    Circle pizza(30);
    // //Circle pizza;
    // //pizza.radius = 30;
    // area = pizza.getArea();
    // cout << "pizza area is " << area << endl;

    // Rectangle r;
    // r.width = 3;
    // r.height = 5;
    // int rArea = r.getArea();
    // cout << "rect area is " << rArea << endl;

    // cout << "I'm in main function" << endl;

    cout << donut.getArea() << endl;

    Circle *p;
    p = &donut;

    cout << p->getArea() << endl;
    cout << (*p).getArea() << endl;

    p = &pizza;
    cout << p->getArea() << endl;
    cout << (*p).getArea() << endl;

    Circle ca[] = {Circle(10), Circle(20), Circle()};
    for (int i = 0; i < 3; i++) {
        cout << "Circle " << i << "의 면적은 " << ca[i].getArea() << endl;
    }

    return 0;
}
