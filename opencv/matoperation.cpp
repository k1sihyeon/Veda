#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int main(void) {

    Mat image = imread("mandrill.jpg", IMREAD_COLOR);
    Mat image2, image3, image4;
    
    image.copyTo(image2);
    image.copyTo(image3);
    image.copyTo(image4);

    image += 50;
    image2 *= 2;
    image3 -= 50;
    image4 /= 2;

    imshow("Mat : Plus", image);
    imshow("Mat : Multiply", image2);
    imshow("Mat : Minus", image3);
    imshow("Mat : Divide", image4);

    waitKey(0);
    return 0;
}