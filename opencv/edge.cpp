#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main() {
    Mat image1 = imread("mandrill.jpg", IMREAD_GRAYSCALE);
    Mat image2 = Mat::zeros(image1.size(), image1.type());
    Mat image3 = Mat::zeros(image1.size(), image1.type());
    Mat image4 = Mat::zeros(image1.size(), image1.type());
    Mat image5 = Mat::zeros(image1.size(), image1.type());

    Mat xEdgeMat, yEdgeMat;
    int ddepth = CV_16S;

    GaussianBlur(image1, image1, Size(3, 3), 0, 0, BORDER_DEFAULT);

    Sobel(image1, xEdgeMat, ddepth, 1, 0);
    Sobel(image1, yEdgeMat, ddepth, 0, 1);
    convertScaleAbs(xEdgeMat, xEdgeMat);
    convertScaleAbs(yEdgeMat, yEdgeMat);
    addWeighted(xEdgeMat, 0.5, yEdgeMat, 0.5, 0, image2);

    Laplacian(image1, image3, ddepth, 3);
    convertScaleAbs(image3, image3);

    Scharr(image1, xEdgeMat, ddepth, 1, 0);
    Scharr(image1, yEdgeMat, ddepth, 0, 1);
    convertScaleAbs(xEdgeMat, xEdgeMat);
    convertScaleAbs(yEdgeMat, yEdgeMat);
    addWeighted(xEdgeMat, 0.5, yEdgeMat, 0.5, 0, image4);

    Canny(image1, image5, 50, 150);

    imshow("Image", image1);
    imshow("Sobel", image2);
    imshow("Laplacian", image3);
    imshow("Scharr", image4);
    imshow("Canny", image5);

    waitKey(0);

    return 0;
}