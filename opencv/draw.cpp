#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

bool isDrawing = false;
Point prevPt;
Mat image;

void CallBackFunc(int event, int x, int y, int flags, void* userdata) {
    Scalar color(0, 0, 0);
    Point newPt(x, y);

    if (event == EVENT_LBUTTONDOWN) {
        isDrawing = true;
        circle(image, newPt, 1, color, -1);
        // -1: 굵기 - 마이너스 값이면 filled로
    }
    else if (event == EVENT_MOUSEMOVE) {
        if (isDrawing)
            line(image, prevPt, newPt, color, 1);
    }
    else if (event == EVENT_LBUTTONUP) {
        isDrawing = false;
        line(image, prevPt, newPt, color, 1);
    }

    prevPt = newPt;
}

int main(void) {

    // 색깔 BGR 순서임
    image = Mat(300, 400, CV_8UC3, Scalar(255, 255, 255));
    namedWindow("Draw", WINDOW_NORMAL);

    setMouseCallback("Draw", CallBackFunc, NULL);

    while (true) {
        imshow("Draw", image);

        int key = waitKey(1) & 0xFF;
        // 키보드 입력을 받고 하위 8비트만 가져옴
        
        if (key == 27)  // esc키 이면
            break;
    }

    destroyAllWindows();

    return 0;
}