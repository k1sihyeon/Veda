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
#define LIMIT_BYTE(n) (n>USHRT_MAX)?USHRT_MAX:(n<0)?0:n

typedef unsigned char ubyte;
typedef unsigned short byte;

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

extern int readBmp(char* filename, ubyte** pData, int* cols, int* rows, int* color);

int main(int argc, char** argv) {
    int cols, rows, color = 24;
    ubyte r, g, b, a = 255;
    ubyte *pData; //, *pBmpData, *pFbMap;
    unsigned short *pBmpData, *pFbMap;
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

    // pBmpData = (ubyte *)malloc(vinfo.xres * vinfo.yres * sizeof(ubyte) * vinfo.bits_per_pixel / 8);
    pData = (ubyte *)malloc(vinfo.xres * vinfo.yres * sizeof(ubyte) * color / 8);
    // pFbMap = (ubyte *)mmap(0, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    
    
    // pBmpData = (unsigned short *)malloc(vinfo.xres * vinfo.yres /* sizeof(unsigned short) */ * vinfo.bits_per_pixel / 8);
    pBmpData = (unsigned short *)malloc(vinfo.xres * vinfo.yres * sizeof(unsigned short) * vinfo.bits_per_pixel / 16);
    pFbMap = (unsigned short *)mmap(0, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);


    if (pFbMap == (unsigned short *) -1) {
        perror("mmap()");
        return -1;
    }

    if (readBmp(argv[1], &pData, &cols, &rows, &color) < 0) {
        perror("readBmp()");
        return -1;
    }

    for (int y = 0, k, total_y; y < rows; y++) {
        k = (rows - y - 1) * cols * color / 8;
        // total_y = y * vinfo.xres * vinfo.bits_per_pixel / 8;
        total_y = y * vinfo.xres * vinfo.bits_per_pixel / 16;
        // 16비트면 16으로 나누거나 사용하지 않기

        for (int x = 0; x < cols; x++) {
            b = LIMIT_UBYTE(pData[k + x*color/8 + 0] + 50);
            g = LIMIT_UBYTE(pData[k + x*color/8 + 1] + 50);
            r = LIMIT_UBYTE(pData[k + x*color/8 + 2] + 50);
            
            unsigned short px = makepixel(r, g, b);
            //px = LIMIT_BYTE(px);
            
            // == *(pBmpData + x + y * vinfo.xres) = px;
			*(pBmpData + x * vinfo.bits_per_pixel / 16 + total_y) = px;		// == *(pBmpData + x + total_y) = px;
			
            // *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 0) = b;
            // *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 1) = g;
            // *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 2) = r;
            // *(pBmpData + x * vinfo.bits_per_pixel / 8 + total_y + 3) = a;
        }
    }

    memcpy(pFbMap, pBmpData, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8);

    munmap(pFbMap, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8);

    free(pBmpData);
    free(pData);

    close(fbfd);

    return 0;
}
