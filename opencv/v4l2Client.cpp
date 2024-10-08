#include <iostream>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <asm/types.h>    

#define FBDEV   "/dev/fb0"

#define PORT    5100
#define WIDTH   800
#define HEIGHT  600

extern inline int clip(int value, int min, int max) {
	return(value > max ? max : value < min ? min : value);
}

int main(int argc, char** argv) {

    int ssock, port = PORT;
    struct sockaddr_in servaddr;

    if (argc < 2) {
        printf("Usage : %s <IP ADRESS>\n", argv[0]);
        return -1;
    }

    if (argc == 3)
        port = atoi(argv[2]);

    // socket 생성 - TCP (STREAM)
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // 서버 주소 초기화
    memset(&servaddr, 0, sizeof(servaddr));
    // 명령어 인수의 string IP주소를 servaddr에 네트워크 형식으로 변환
    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    // 접속
    if (connect(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    // init fb
    int fbfd = -1;
    struct fb_var_screeninfo vinfo;
    unsigned short* fbp;

    fbfd = open(FBDEV, O_RDWR);
    if (fbfd == -1) {
        perror("open() : framebuffer device");
        return EXIT_FAILURE;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        return EXIT_FAILURE;
    }

    long screensize = vinfo.xres * vinfo.yres * 2;
    fbp = (unsigned short *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if (fbp == (unsigned short *)-1) {
        perror("mmap() : framebuffer device to memory");
        return EXIT_FAILURE;
    }

    memset(fbp, 0, screensize);

    printf("I'm going to read img!\n");

    while (true) {

        unsigned char* inimg;

        size_t total_received = 0;
        size_t remain = 960000;

        while (remain > 0) {
            size_t to_receive = (remain < BUFSIZ) ? remain : BUFSIZ;
            //size_t received = recv(ssock, inimg + total_received, to_receive, 0);
            size_t received = read(ssock, inimg + total_received, to_receive);

            //printf("left : %d\n", remain);

            if (received <= 0) {
                perror("Error : recv()");
                break;
            }

            total_received += received;
            remain -= received;
        }

        printf("receive done!!\n");

        int width = WIDTH;
        int height = HEIGHT;
        int istride = width * 2;

        int x, y, j;
        int y0, u, y1, v, r, g, b;
        unsigned short pixel;
        long location = 0;

        for (y = 0; y < height; ++y) {
            for (j = 0, x = 0; j < vinfo.xres * 2; j += 4, x += 2) {
                if (j >= width * 2) {
                    /* 현재의 화면에서 이미지를 넘어서는 빈 공간을 처리 */
                    location++;
                    location++;
                    continue;
                }

                /* YUYV 성분을 분리 */
                y0 = inimg[j];
                u = inimg[j + 1] - 128;
                y1 = inimg[j + 2];
                v = inimg[j + 3] - 128;

                /* YUV를 RGB로 전환 */
                r = clip((298 * y0 + 409 * v + 128) >> 8, 0, 255);
                g = clip((298 * y0 - 100 * u - 208 * v + 128) >> 8, 0, 255);
                b = clip((298 * y0 + 516 * u + 128) >> 8, 0, 255);
                pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

                /* 16비트 컬러로 전환 */
                fbp[location++] = pixel;

                /* YUV를 RGB로 전환 */
                r = clip((298 * y1 + 409 * v + 128) >> 8, 0, 255);
                g = clip((298 * y1 - 100 * u - 208 * v + 128) >> 8, 0, 255);
                b = clip((298 * y1 + 516 * u + 128) >> 8, 0, 255);
                pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

                /* 16비트 컬러로 전환 */
                fbp[location++] = pixel;
            };
            inimg += istride;
        };
    }

    munmap(fbp, screensize);
    close(fbfd);

    return 0;
}