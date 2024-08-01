#include <bits/stdc++.h>
using namespace std;

int main(void) {

    [](int x, int y) {
        cout << "합은 " << x + y;
    } (2, 3);

    cout << endl;

    auto love = [] (string a, string b) {
        cout << a << " 보다 " << b << "(이)가 좋아 " << endl;
    };
    love("돈", "너");
    love("noodle", "dumpling");

    double pi = 3.141592;
    auto calc = [pi](int r) -> double {
        return pi * r * r;
    };
    cout << "면적은 " << calc(3) << endl;

    int sum = 0;
    [&sum](int x, int y) {
        sum = x + y;
    } (2, 3);
    cout << "합은 " << sum << endl;


    return 0;
}