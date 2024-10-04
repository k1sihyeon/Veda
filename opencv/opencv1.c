#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgcodecs/imgcodecs_c.h>

#pragma comment(lib, "cv.lib")
#pragma comment(lib, "highgui.lib")
#pragma comment(lib, "cxcore.lib")

int main(int argc, char **argv) {

    IplImage *image1 = cvLoadImage("sample1.jpg", CV_LOAD_IMAGE_COLOR);
    IplImage *image2 = cvLoadImage("sample2.jpg", CV_LOAD_IMAGE_COLOR);

    IplImage *image_add = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);
    IplImage *image_sub = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);
    IplImage *image_mul = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);
    IplImage *image_div = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);

    IplImage *image_gray1 = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
    IplImage *image_gray2 = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
    IplImage *image_white = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
    IplImage *image_gray_sub = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);

    cvNamedWindow("IMAGE_1", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_2", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_ADDITION", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_SUBSTRACTION", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_MULTIPLICATION", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_DIVISION", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_GRAY1", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_GRAY2", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("IMAGE_WHITE", CV_WINDOW_AUTOSIZE);

    cvAdd(image1, image2, image_add, NULL);
    cvSub(image1, image2, image_sub, NULL);
    cvMul(image1, image2, image_mul, 1);
    cvDiv(image1, image2, image_div, 1);

    cvCvtColor(image1, image_gray1, CV_RGB2GRAY);
    cvCvtColor(image2, image_gray2, CV_RGB2GRAY);
    cvAbsDiff(image_gray1, image_gray2, image_gray_sub);    // 행렬의 차 계산
    cvThreshold(image_gray_sub, image_white, 100, 255, CV_THRESH_BINARY);   // threshold 계산

    cvShowImage("IMAGE_1", image1);
    cvShowImage("IMAGE_2", image2);
    cvShowImage("IMAGE_ADDITION", image_add);
    cvShowImage("IMAGE_SUBSTRACTION", image_sub);
    cvShowImage("IMAGE_MULTIPLICATION", image_mul);
    cvShowImage("IMAGE_DIVISION", image_div);

    cvShowImage("IMAGE_GRAY1", image_gray1);
    cvShowImage("IMAGE_GRAY2", image_gray2);
    cvShowImage("IMAGE_WHITE", image_white);

    cvWaitKey(0);

    cvReleaseImage(&image1);
    cvReleaseImage(&image2);
    cvReleaseImage(&image_add);
    cvReleaseImage(&image_sub);
    cvReleaseImage(&image_mul);
    cvReleaseImage(&image_div);
    cvReleaseImage(&image_gray1);
    cvReleaseImage(&image_gray2);
    cvReleaseImage(&image_white);
    cvReleaseImage(&image_gray_sub);

    cvDestroyAllWindows();

    return 0;
}