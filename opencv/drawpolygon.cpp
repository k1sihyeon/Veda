#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

using namespace cv;

int main() {

    Mat image = Mat::zeros(300, 400, CV_8UC3);
    image.setTo(cv::Scalar(255, 255, 255));

    Mat image2 = Mat::zeros(300, 400, CV_8UC3);
    image2.setTo(cv::Scalar(255, 255, 255));

    Mat image3 = Mat::zeros(300, 400, CV_8UC3);
    image3.setTo(cv::Scalar(255, 255, 255));

    Scalar color(255, 0, 255);
    Point p1(50, 50), p5(150, 150);

    std::vector<Point> contour;
    contour.push_back(p1);
    contour.push_back(Point(200, 100));
    contour.push_back(Point(250, 50));
    contour.push_back(Point(180, 200));
    contour.push_back(p5);

    const Point *pts = (const Point *) Mat(contour).data;
    int npts = (int)contour.size();
    
    // 1: contour 개수
    polylines(image, &pts, &npts, 1, true, color);  // 외곽 선
    // bool isClosed: true -> 닫힌 도형, false -> 열린 도형
    polylines(image2, &pts, &npts, 1, false, color);

    fillPoly(image3, &pts, &npts, 1, color);

    imshow("Draw Polygon", image);
    imshow("Draw Polygon 1", image2);
    imshow("Draw Polygon 2", image3);

    waitKey(0);

    return 0;
}