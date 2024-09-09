#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#define FBDEVICE "/dev/fb0"

typedef unsigned char ubyte;

struct fb_var_screeninfo vinfo;

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

// 점 그리기
static void drawpoint(int fd, int x, int y, ubyte r, ubyte g, ubyte b) {
    ubyte a = 0xFF;

    // offset = (x_loc + y_loc * 해상도 넓이) * (fb_var_screeninfo.bits_per_pixel / 10)
    int offset = (x + y * vinfo.xres) * vinfo.bits_per_pixel / 8. ;

    lseek(fd, offset, SEEK_SET);

    unsigned short px = makepixel(r, g, b);
    write(fd, &px, 2);

    // write(fd, &b, 1);
    // write(fd, &r, 1);
    // write(fd, &g, 1);
    // write(fd, &a, 1);
}

int main(int argc, char** argv) {
    int fbfd, status, offset;
    
    unsigned short pixel;

    fbfd = open(FBDEVICE, O_RDWR);
    if (fbfd < 0) {
        perror("Error : cannot open framebuffer device");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Error reading fixed infromation");
        return -1;
    }

    drawpoint(fbfd, 50, 50, 255, 0, 0);
    drawpoint(fbfd, 100, 100, 0, 255, 0);
    drawpoint(fbfd, 150, 150, 0, 0, 255);
    
    close(fbfd);

    return 0;
}
