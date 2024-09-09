#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "bmpHeader.h"

#define FBDEVFILE   "/dev/fb0"

#define LIMIT_UBYTE(n) (n>UCHAR_MAX)?UCHAR_MAX:(n<0)?0:n

typedef unsigned char ubyte;

extern int readBmp(char* filename, ubyte** pData, int* cols, int* rows, int* color);

int main(int argc, char** argv) {
    int cols, rows, color = 24;
    ubyte r, g, b, a = 255;
    ubyte *pData, *pBmpData, *pFbMap;
    struct fb_var_screeninfo vinfo;
    int fbfd;

    if (argc != 2) {
        printf("Usage : ./%s <bmpfile.bmp>\n", argv[0]);
        return 0;
    }

    fbfd = open(FBDEVFILE, O_RDWR);
    if (fbfd < 0) {
        perror("open()");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl() : FBIOGET_VSCREENINFO");
        return -1;
    }

    pBmpData = (ubyte *)malloc(vinfo.xres * vinfo.yres * sizeof(ubyte) * vinfo.bits_per_pixel / 8);
    pData = (ubyte *)malloc(vinfo.xres * vinfo.yres * sizeof(ubyte) * color / 8);
    pFbMap = (ubyte *)mmap(0, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if ((unsigned) pFbMap == (unsigned) -1) {
        perror("mmap()");
        return -1;
    }

    if (readBmp(argv[1], &pData, &cols, &rows, &color) < 0) {
        perror("readBmp()");
        return -1;
    }

    for (int y = 0, k, total_y; y < rows; y++) {
        k = (rows - y - 1) * cols * color / 8;
        total_y = y * vinfo.xres * vinfo.bits_per_pixel / 8;

        for (int x = 0; x < cols; x++) {
            b = LIMIT_UBYTE(pData[k + x*color/8 + 0]);
            g = LIMIT_UBYTE(pData[k + x*color/8 + 1]);
            r = LIMIT_UBYTE(pData[k + x*color/8 + 2]);

            *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 0) = b;
            *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 1) = g;
            *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 2) = r;
            *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 3) = a;
        }
    }

    memcpy(pFbMap, pBmpData, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8);

    munmap(pFbMap, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8);

    free(pBmpData);
    free(pData);

    close(fbfd);

    return 0;
}