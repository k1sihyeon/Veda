#include <fcntl.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* low-level i/o */
#include <asm/types.h>
#include <errno.h>
#include <linux/fb.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

/* for videodev2.h */
#include <linux/videodev2.h>

#define VIDEODEV "/dev/video0"
#define WIDTH 	800
#define HEIGHT 	600

#define PORT 	5100
#define MAX_CLIENT 10


struct buffer {
    void* start;		// 아마 struct v4l2_buffer?
    size_t length;
};

struct buffer *buffers        = NULL;
static unsigned int n_buffers = 0;
static struct fb_var_screeninfo vinfo;

int main(int argc, char** argv) {
    
}