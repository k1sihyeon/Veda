#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>

#define FBDEVICE "/dev/fb0"

typedef unsigned char ubyte;

struct fb_var_screeninfo vinfo;

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

// 점 그리기
static void drawpoint(int fd, int x, int y, ubyte r, ubyte g, ubyte b) {
    //ubyte a = 0xFF;

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

// 선 그리기
static void drawline(int fd, int start_x, int end_x, int y, ubyte r, ubyte g, ubyte b) {
    //ubyte a = 0xFF;

    for (int x = start_x; x < end_x; x++) {
        int offset = (x + y * vinfo.xres) * vinfo.bits_per_pixel / 8. ;
        lseek(fd, offset, SEEK_SET);

        unsigned short px = makepixel(r, g, b);
        write(fd, &px, sizeof(px));
    }
}

// 선 그리기 - Bresenham
static void drawline_br(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
    //ubyte a = 0xFF;

    int dy = end_y - start_y;
    int dx = end_x - start_x;

    int d = 2 * dy - dx;
    int x = start_x;
    int y = start_y;

    while (x <= end_x) {
        int offset = (x + y * vinfo.xres) * vinfo.bits_per_pixel / 8. ;
        lseek(fd, offset, SEEK_SET);

        unsigned short px = makepixel(r, g, b);
        write(fd, &px, sizeof(px));

        x += 1;

        if (d < 0)
            d += 2 * dy;
        else {
            d += 2 * (dy - dx);
            y += 1;
        }
    }
}

static void circlePlot(int fd, int cx, int cy, int x, int y, ubyte r, ubyte g, ubyte b) {
    drawpoint(fd, cx + x, cy + y, r, g, b);
    drawpoint(fd, cx + y, cy + x, r, g, b);
    drawpoint(fd, cx - x, cy + y, r, g, b);
    drawpoint(fd, cx - y, cy + x, r, g, b);
    drawpoint(fd, cx - x, cy - y, r, g, b);
    drawpoint(fd, cx - y, cy - x, r, g, b);
    drawpoint(fd, cx + x, cy - y, r, g, b);
    drawpoint(fd, cx + y, cy - x, r, g, b);
}

static void drawcircle(int fd, int cx, int cy, int radius, ubyte r, ubyte g, ubyte b) {
    int x = 0;
    int y = radius;
    int p = 1 - radius;

    circlePlot(fd, cx, cy, x, y, r, g, b);

    while (x <= y) {
        x += 1;

        if (p < 0)
            p += 2 * x + 1;
        else {
            y -= 1;
            p += 2 * (x - y) + 1;
        }

        circlePlot(fd, cx, cy, x, y, r, g, b);
    }
}

static void drawface(int fd, int sx, int sy, int ex, int ey, ubyte r, ubyte g, ubyte b) {
    ubyte a = 0xFF;

    if (ex == 0)
        ex = vinfo.xres;
    if (ey == 0)
        ey = vinfo.yres;

    for (int x = sx; x < ex; x++) {
        for (int y = sy; y < ey; y++) {

            drawpoint(fd, x, y, r, g, b);
            // int offset = (x + y * vinfo.xres) * vinfo.bits_per_pixel / 8. ;
            // lseek(fd, offset, SEEK_SET);
            // unsigned short px = makepixel(r, g, b);
            // write(fd, &px, sizeof(px));
        }
    }
}

// 여기서는 16비트임!! -> unsigned short로!!!!
static void drawfacemmap(int fd, int sx, int sy, int ex, int ey, ubyte r, ubyte g, ubyte b) {
    ubyte a = 0xFF;
    unsigned short* pfb;
    int color = vinfo.bits_per_pixel / 8.;

    if (ex == 0)
        ex = vinfo.xres;
    if (ey == 0)
        ey = vinfo.yres;
	
	unsigned short px = makepixel(r, g, b);

    pfb = (unsigned short *)mmap(0, vinfo.xres * vinfo.yres * color, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for (int x = sx; x < ex; x++) {
        for (int y = sy; y < ey; y++) {
            // *(pfb + (x + 0) + y * vinfo.xres * color) = b;
            // *(pfb + (x + 1) + y * vinfo.xres * color) = g;
            // *(pfb + (x + 2) + y * vinfo.xres * color) = r;
            // *(pfb + (x + 3) + y * vinfo.xres * color) = a;
	
            *(pfb + x + y * vinfo.xres) = px;
           // pfb[(x * color + 0) + y * vinfo.xres * color] = b;
           // pfb[(x * color + 1) + y * vinfo.xres * color] = g;
           // pfb[(x * color + 2) + y * vinfo.xres * color] = r;
           // pfb[(x * color + 3) + y * vinfo.xres * color] = a;
        }
    }

    munmap(pfb, vinfo.xres * vinfo.yres * color);
}

void lineBres(int fd, int x1, int y1, int x2, int y2, ubyte r, ubyte g, ubyte b) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int p, twoDy, twoDyDx;
    int twoDx, twoDxDy;
    int x, y, xEnd, yEnd, IorD;

    if (dx > dy) {
        p = 2 * dy - dx;          // 초기값 p1
        twoDy = 2 * dy;           // pk < 0 일때
        twoDyDx = 2 * (dy - dx);  // pk >= 0 일때

        if (x1 > x2)  // dx < 0
        {
            x = x2;
            y = y2;
            xEnd = x1;

            if (y1 - y2 > 0)
                IorD = 1;
            else
                IorD = -1;
        }
        else  // dx >= 0
        {
            x = x1;
            y = y1;
            xEnd = x2;

            if (y2 - y1 > 0)
                IorD = 1;
            else
                IorD = -1;
        }

        // start point marking with a user-defined color(r,g,b)
        drawpoint(fd, x, y, r, g, b);

        while (x < xEnd) {
            x += 1;  // x 1증가

            if (p < 0) {
                p += twoDy;
            }
            else {
                y += IorD;
                p += twoDyDx;
            }

            drawpoint(fd, x, y, r, g, b);
        }
    }

    else  // dy > dx
    {
        p = 2 * dx - dy;
        twoDx = 2 * dx;
        twoDxDy = 2 * (dx - dy);

        if (y1 > y2) {
            x = x2;
            y = y2;
            yEnd = y1;

            if (x1 - x2 > 0)
                IorD = 1;
            else
                IorD = -1;
        }
        else {
            x = x1;
            y = y1;
            yEnd = y2;

            if (x2 - x1 > 0)
                IorD = 1;
            else
                IorD = -1;
        }

        // start point marking with a user-defined color(r,g,b)
        drawpoint(fd, x, y, r, g, b);

        while (y < yEnd) {
            y += 1;
            if (p < 0)
                p += twoDx;
            else {
                x += IorD;
                p += twoDxDy;
            }

            drawpoint(fd, x, y, r, g, b);
        }
    }
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

    // drawpoint(fbfd, 50, 50, 255, 0, 0);
    // drawpoint(fbfd, 100, 100, 0, 255, 0);
    // drawpoint(fbfd, 150, 150, 0, 0, 255);

    // drawline(fbfd, 0, 100, 200, 0, 255, 255);

    // // drawface(fbfd, 500, 500, 700, 700, 255, 0, 0);
    // // drawface(fbfd, 700, 500, 900, 700, 0, 255, 0);
    // // drawface(fbfd, 900, 500, 1100, 700, 0, 0, 255);

    // // drawface(fbfd, 500, 300, 700, 500, 255, 0, 255);
    // // drawface(fbfd, 900, 300, 1100, 500, 0, 255, 255);

    // lineBres(fbfd, 400, 400, 600, 700, 0, 255, 255);
    // lineBres(fbfd, 500, 350, 600, 600, 255, 0, 255);

    // drawcircle(fbfd, 500, 500, 200, 255, 255, 0);
    // drawcircle(fbfd, 900, 500, 200, 255, 255, 255);

    // lineBres(fbfd, 300, 300, 1300, 1100, 255, 255, 0);

    // lineBres(fbfd, 100, 100, 1300, 700, 0, 255, 255);

    // 

    drawfacemmap(fbfd, 200, 300, 400, 600, 0, 0, 255);
    drawfacemmap(fbfd, 400, 300, 600, 600, 255, 255, 255);
    drawfacemmap(fbfd, 600, 300, 800, 600, 255, 10, 10);
   // drawfacemmap(fbfd, 800, 300, 1000, 500, 0, 0, 0);

    // drawfacemmap(fbfd, 0, 0, 100, 100, 0, 0, 255);
    // drawfacemmap(fbfd, 100, 100, 200, 200, 255, 255, 255);
    
    close(fbfd);

    return 0;
}
