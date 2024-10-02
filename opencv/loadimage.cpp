#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int main(void) {
    Mat image = imread("sample.jpg", IMREAD_COLOR);
    imshow("Load Image 1", image);

    Mat image2 = imread("sample.png", IMREAD_COLOR);
    imshow("Load Image 2", image2);

    waitKey(0);
    return 0;
}