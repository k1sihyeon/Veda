#include <bits/stdc++.h>

using namespace std;

class Point {
   public:
    Point(const Point &pt) : _x(pt._x), _y(pt._y) {
        cout << "복사 생성자 : " << _x << ", " << _y << endl;
    }

    Point(int x = 0, int y = 0) : _x(x), _y(y) {
        cout << "생성자1 : " << _x << ", " << _y << endl;
    }
    
    Point(const Point &pt, int x, int y) {
        _x = pt._x + x;
        _y = pt._y + y;
        cout << "생성자2 : " << _x << ", " << _y << endl;
    }
    
    ~Point() {
        cout << "소멸자 : " << _x << ", " << _y << endl;
    }
    
    void Print() {
        cout << "(" << _x << ", " << _y << ")" << endl;
    }

   private:
    int _x;
    int _y;
};

// 임시객체 생성 및 반환
Point GetPoint(Point pt) {
    return Point(pt, 2, 2);
}

int main() {
    Point pt1 = GetPoint(Point(1, 1));  // GetPoint 함수 호출
    Point pt2 = Point(100, 100);        // 임시객체 생성, P2 초기화
    //Point &pt3 = Point(200, 200);       // 임시객체 생성, P3이 참조
    Point pt4;                          // 일반 생성
    pt4 = Point(300, 300);              // 임시객체 생성 및 대입

    pt1.Print();
    pt2.Print();
    //pt3.Print();
    pt4.Print();
    Point(300, 300).Print();  // 임시객체 생성 & 멤버 함수 호출

    cout << "프로그램 종료" << endl;
    
    return 0;
}

// int main(void) {
//     cout << "입력할 정수의 개수는? ";
//     int n;
//     cin >> n;

//     if (n <= 0)
//         return 0;

//     int *p = new int[n];
//     if (!p) {
//         cout << "메모리를 할당할 수 없습니다.";
//         return 0;
//     }

//     for (int i = 0; i < n; i++) {
//         cout << i + 1 << "번째 정수: ";
//         cin >> p[i];
//     }

//     int sum = 0;
//     for (int i = 0; i < n; i++)
//         sum += p[i];

//     cout << "평균  = " << sum / n << endl;

//     return 0;
// }