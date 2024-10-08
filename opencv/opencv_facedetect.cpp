#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <vector>
#include <iostream>

#define FBDEV   "/dev/fb0"
#define CAM_COUNT   100
#define CAM_WIDTH   640
#define CAM_HEIGHT  480

using namespace cv;

const static char* cascade_name = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml";

typedef unsigned char ubyte;

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

int main(int argc, char** argv) {
    int fbfd;
    struct fb_var_screeninfo vinfo;

    unsigned char *buffer;
    unsigned short *pfbmap;
    unsigned int x, y, i, j, screensize;

    VideoCapture vc(0);
    CascadeClassifier cascade;      // 얼굴 인식 classifier
    Mat frame(CAM_WIDTH, CAM_HEIGHT, CV_8UC3, Scalar(255));
    Point pt1, pt2;                 // 인식된 얼굴의 대각선의 두 점을 저장하기 위한 변수

    if (!cascade.load(cascade_name)) {
        perror("load()");
        return EXIT_FAILURE;
    }

    // 카메라 캡쳐 속성 설정
    vc.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
    vc.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

    // 프레임 버퍼 디바이스 열기
    fbfd = open(FBDEV, O_RDWR);
    if (fbfd == -1) {
        perror("open() : framebuffer device");
        return EXIT_FAILURE;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information.");
        return EXIT_FAILURE;
    }

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8.;
    pfbmap = (unsigned short *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if (pfbmap == (unsigned short *)-1) {
        perror("mmap() : framebuffer device to memory");
        return EXIT_FAILURE;
    }

    memset(pfbmap, 0, screensize);

    // 출력
    for (i = 0; i < CAM_COUNT; i++) {
        int colors = vinfo.bits_per_pixel / 8;
        long location = 0;
        int istride = frame.cols * colors;

        // 카메라의 프레임 획득
        vc >> frame;

        // 얼굴 인식
        Mat image(CAM_WIDTH, CAM_HEIGHT, CV_8UC1, Scalar(255));
        cvtColor(frame, image, COLOR_BGR2GRAY);
        equalizeHist(image, image);

        std::vector<Rect> faces;
        cascade.detectMultiScale(image, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

        // if (!faces.empty()) {
        //     std::cout << "face detected!\n";
        // }

        for (j = 0; j < faces.size(); j++) {
            pt1.x = faces[j].x;
            pt1.y = faces[j].y;

            pt2.x = (faces[j].x + faces[j].width);
            pt2.y = (faces[j].y + faces[j].height);

            cv::rectangle(frame, pt1, pt2, Scalar(255, 0, 0), 3, 8);
        }

        // Mat 영상 데이터 획득
        buffer = (uchar*)frame.data;

        ubyte r, g, b;

        for (y = 0, location = 0; y < frame.rows; y++) {
            for (x = 0; x < vinfo.xres; x++) {
                if (x >= frame.cols) {
                    location++;
                    continue;
                }

                b = buffer[(y * image.cols + x) * 3 + 0];
                g = buffer[(y * image.cols + x) * 3 + 1];
                r = buffer[(y * image.cols + x) * 3 + 2];

                unsigned short px = makepixel(r, g, b);

                pfbmap[location++] = px;
            }
        }
    }

    munmap(pfbmap, screensize);

    close(fbfd);
    return 0;
}