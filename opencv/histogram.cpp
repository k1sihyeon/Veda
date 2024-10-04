#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main() {
    MatND hist;
    const int* chan_nos = {};
    float chan_range[] = {0.0, 255.0};
    const float* chan_ranges = chan_range;
    int histSize = 255;
    double maxVal = 0, minVal = 0;

    Mat image1 = imread("mandrill.jpg", IMREAD_GRAYSCALE);
    Mat image2 = Mat::zeros(image1.size(), image1.type());
    Mat histImg1(histSize, histSize, CV_8U, Scalar(histSize));
    Mat histImg2(histSize, histSize, CV_8U, Scalar(histSize));
    
    image2 = image1 + 50;

    calcHist(
        &image1, 1,     // 단일 영상의 히스토그램만
        chan_nos,       // 대상 채널
        Mat(),          // 마스크 없음
        hist,           // 결과 저장
        1,              // 1D 차원 히스토그램
        &histSize,      // 빈도 수
        &chan_ranges    // 화소 값 범위
    );

    minMaxLoc(hist, &minVal, &maxVal, 0, 0);
    int hpt = static_cast<int>(0.9 * histSize);     // 90%를 최대점으로 설정

    for (int h = 0; h < histSize; h++) {
        float binVal = hist.at<float>(h);
        int intensity = static_cast<int>(binVal * hpt / maxVal);

        line(histImg1, Point(h, histSize), Point(h, histSize - intensity), Scalar::all(0));
    }

    calcHist(&image2, 1, chan_nos, Mat(), hist, 1, &histSize, &chan_ranges);

    minMaxLoc(hist, &minVal, &maxVal, 0, 0);

    for (int h = 0; h < histSize; h++) {
        float binVal = hist.at<float>(h);
        int intensity = static_cast<int>(binVal * hpt / maxVal);
        line(histImg2, Point(h, histSize), Point(h, histSize - intensity), Scalar::all(0));
    }

    imshow("Historgram1", histImg1);
    imshow("Histogram2", histImg2);

    waitKey(0);

    return 0;
}