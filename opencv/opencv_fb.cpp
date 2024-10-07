//LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libcamera/v4l2-compat.so ./build/opencv_fb

#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define FBDEV           "/dev/fb0"
#define CAM_COUNT    100
#define CAM_WIDTH    640
#define CAM_HEIGHT   480

#define LIMIT_UBYTE(n) (n > UCHAR_MAX) ? UCHAR_MAX : (n < 0) ? 0 : n

typedef unsigned char ubyte;

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

int main(int argc, char** argv) {
    int fbfd;
    struct fb_var_screeninfo vinfo;

    unsigned char *buffer;
    unsigned short *pfbmap;
    unsigned int x, y, i, screensize;

    VideoCapture vc(0);
    Mat image(CAM_WIDTH, CAM_HEIGHT, CV_8UC3, Scalar(255));
    if (!vc.isOpened()) {
        perror("OpenCV : open WebCam\n");
        return EXIT_FAILURE;
    }

    // 캡쳐할 영상의 속성 설정
    vc.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
    vc.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

    // 프레임 버퍼 열기
    fbfd = open(FBDEV, O_RDWR);
    if (fbfd == -1) {
        perror("open() : framebuffer device\n");
        return EXIT_FAILURE;
    }

    // 프레임 버퍼 정보 가져오기
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information\n");
        return EXIT_FAILURE;
    }

    // 프레임 버퍼 공간 확보
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8.;
    pfbmap = (unsigned short *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (pfbmap == (unsigned short *)-1) {
        perror("mmap() : framebuffer device to memory");
        return EXIT_FAILURE;
    }
    
    // 출력
    memset(pfbmap, 0, screensize);

    for (i = 0; i < CAM_COUNT; i++) {
        int colors = vinfo.bits_per_pixel / 8;
        long location = 0;
        int istride = image.cols * colors;

        ubyte r, g, b;
        
        vc >> image;
        buffer = (uchar *)image.data;

        for (y = 0, location = 0; y < image.rows; y++) {
            for (x = 0; x < vinfo.xres; x++) {

                // 화면의 가로 xres 상에서 이미지의 가로를 넘으면
                // 값을 쓰지 않고 index만 증가 시킴
                if (x >= image.cols) {
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