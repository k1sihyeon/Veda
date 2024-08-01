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

    double getArea() {
        return radius * radius * 3.14;
    }

    void setRadius(int r) {
        radius = r;
    }

    int getRadius() {
        return radius;
    }

    Circle();
    Circle(int r) {
        radius = r;
        cout << "Circle created; radius = " << radius << endl;
    }

    Circle (const Circle& c) {
		this->radius = c.radius;
        cout << "copy constructor; radius = " << radius << endl;
	}

    ~Circle() {
        cout << "Circle Deleted! radius = " << radius << endl;
    }
};

Circle::Circle() : Circle(1) { }

void increase(Circle& c) {
    c.setRadius(c.getRadius() + 1);
}

Circle getCircle() {
    Circle tmp(30);
    return tmp;
}

void swapCircle(Circle& a, Circle& b) {
    Circle tmp = a;
    a = b;
    b = tmp;
}

void readRadius(Circle &c) {
    int r;
    cout << "input >> ";
    cin >> r;
    c.setRadius(r);
}

int main(int argc, char** argv, char** environ) {

    Circle waffle(30);
    increase(waffle);
    cout << waffle.getRadius() << endl;

    Circle dessert(waffle);

    // Circle c;
    // Circle& ref = c;
    // ref.setRadius(10);
    // cout << ref.getArea() << " " << c.getArea() << endl;

    return 0;
}
