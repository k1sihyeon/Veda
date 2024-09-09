#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#define FBDEVICE    "/dev/fb0"

int main(int argc, char** argv) {
    int fbfd = 0;
    
    // 프레임 버퍼 정보 처리 구조체
    struct fb_var_screeninfo vinfo, old_vinfo;
    struct fb_fix_screeninfo finfo;

    // 프레임 버퍼 디바이스 열기
    fbfd = open(FBDEVICE, O_RDWR);
    if (fbfd < 0) {
        perror("Error : connot open framebuffer device");
        return -1;
    }

    // 현재 프레임 버퍼에 대한 화면 정보 가져오기
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("Error reading fixed information");
        return -1;
    }

    // 현재 프레임 버퍼에 대한 가상화면 정보 가져오기
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Error reading variable information");
        return -1;
    }

    // frame buffer 정보 출력
    printf("Resolution : %d x %d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    printf("Virtual Resolution : %d x %d\n", vinfo.xres_virtual, vinfo.yres_virtual);
    printf("Length of frame buffer memory : %d\n", finfo.smem_len);

    old_vinfo = vinfo;

    // 프레임 버퍼에 새로운 해상도 설정
    vinfo.xres = 800;
    vinfo.yres = 600;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        perror("fbdev ioctl(PUT)");
        return -1;
    }

    // 설정한 새로운 프레임 버퍼 정보 출력
    printf("New Resolution : %d x %d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    getchar();

    // 이전 값으로 돌리기
    ioctl(fbfd, FBIOPUT_VSCREENINFO, &old_vinfo);

    close(fbfd);

    return 0;
}