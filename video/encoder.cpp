#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/videodev2.h>

#define VIDEO_DEV "/dev/video0"
#define WIDTH 	800
#define HEIGHT 	600

typedef unsigned char uchar;

struct buffer *buffers = NULL;
static unsigned int n_buffers = 0;  // 한번에 받아온 (req.size) 버퍼 개수
static struct fb_var_screeninfo vinfo;
static int cond = 1;

struct buffer {
    void* start;
    size_t length;
};

static inline int clip(int value, int min, int max) {
    return (value > max ? max : value < min ? min : value);
}

static void sigHandler(int signo) {
    cond = 0;
}

static int read_frame(int fd);
static void process_image(const void* p);

void yuyv2rgb565(uchar* yuyv, ushort* rgb, int width, int height);
int initFramebuffer(ushort** fbPtr, int* size);
static int init_v4l2(int* fd, struct buffer* buffers);

int main(int argc, char** argv) {

    int camfd = -1;

    ///////////////////////////
    // initdevice
	// 카메라 장치 열기
    camfd = open(VIDEO_DEV, O_RDWR | O_NONBLOCK, 0);

    if (-1 == camfd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", VIDEO_DEV, errno, strerror(errno));
        return EXIT_FAILURE;
    }

    // 카메라 장치 초기화 - init_device
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format format;
    unsigned int min;

    // query cap으로 캡쳐/스트리밍 가능 여부 get
    if (xioctl(camfd, VIDIOC_QUERYCAP, &cap) == -1) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n", VIDEO_DEV);
            exit(EXIT_FAILURE);
        }
        else
            mesg_exit("VIDEO_QUERYCAP");
    }

    // 캡쳐 불가능 한 경우 에러 처리
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", VIDEO_DEV);
        exit(EXIT_FAILURE);
    }

    // 스트리밍 불가능 한 경우 에러 처리
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n", VIDEO_DEV);
        exit(EXIT_FAILURE);
    }

    // 비디오 input, standart, tune 등을 설정
    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // VIDIOC_S_CROP
    if (xioctl(camfd, VIDIOC_CROPCAP, &cropcap) == 0) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        /* reset to default */
        xioctl(camfd, VIDIOC_S_CROP, &crop);
    }

    // VIDIOC_S_FMT
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = WIDTH;
    format.fmt.pix.height = HEIGHT;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_NONE;

    if (xioctl(camfd, VIDIOC_S_FMT, &format) == -1) 
        mesg_exit("VIDEO_S_FMT");

    // 아마 드라이버 오류 처리??
    min = format.fmt.pix.width * 2;
    if (format.fmt.pix.bytesperline < min)
        format.fmt.pix.bytesperline = min;

    min = format.fmt.pix.bytesperline * format.fmt.pix.height;
    if (format.fmt.pix.sizeimage < min)
        format.fmt.pix.sizeimage = min;
    
    //////////////////////////////
    // 버퍼 요청 - init_mmap
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count   = 4;              // 4?
    req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory  = V4L2_MEMORY_MMAP;

    // 버퍼 요청 - VIDIOC_REQBUFS
    if (xioctl(camfd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", VIDEO_DEV);
			exit(EXIT_FAILURE);
        }
        else
            mesg_exit("VIDIOC_REQBUFS");
    }

    // 요청 오류 처리
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", VIDEO_DEV);
		exit(EXIT_FAILURE);
    }

    // buffer 저장할 buffers 배열? 메모리 할당
    buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

    // 메모리 할당 오류 처리
    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    // req.count 만큼 받아온 buffer들을 배열에 넣기
    for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

        // VIDIOC_QUERYBUF : 버퍼 요청
        if (xioctl(camfd, VIDIOC_QUERYBUF, &buf))
            mesg_exit("VIDIOC_QUERYBUF");
        
        // 배열 삽입 : 버퍼를 유저공간에 mapping
        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, camfd, buf.m.offset);

        // 배열 삽입 오류 처리
        if (buffers[n_buffers].start == MAP_FAILED)
            mesg_exit("mmap");
    }

    ////////////////////////////
    // start capturing
    for (int i = 0; i < n_buffers; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory  = V4L2_MEMORY_MMAP;
        buf.index   = i;

        // 프레임 요청
        if (xioctl(camfd, VIDIOC_QBUF, &buf) == -1)
            mesg_exit("VIDIOC_QBUF");
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // 스트림 켜기
    if (xioctl(camfd, VIDIOC_STREAMON, &type))
        mesg_exit("VIDIOC_STREAMON");
    
    // while(true) {
    //     read_frame(camfd);
    // }

    /////////////////////
    // mainloop - polling wait
    while(true) {
        while(true) {
            fd_set fds;
            struct timeval tv;
            FD_ZERO(&fds);
            FD_SET(camfd, &fds);

            tv.tv_sec = 2;
            tv.tv_usec = 0;

            int r = select(camfd + 1, &fds, NULL, NULL, &tv);
            if(r == -1) {
				if(EINTR == errno) 
                    continue;
				mesg_exit("select");
			}
            else if(r == 0) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

            // polling으로 읽을 프레임이 있을 때만 읽음
            if (read_frame(camfd))
                break;
        }
    }

    return 0;
}

///////////////////

static int xioctl(int fd, int request, void* arg) {
    int r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

static int read_frame(int fd) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory  = V4L2_MEMORY_MMAP;

    //VIDIOC_DQBUF - 새 프레임의 인덱스를 가져옴
    if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        switch(errno) {
			case EAGAIN:
                return 0;
			case EIO: default: //Could ignore EIO, see spec.
                mesg_exit("VIDIOC_DQBUF");
		}
    }

    std::cout << "buf.length: " << buf.length << ", buf.bytesused: " << buf.bytesused << "\n";

    // index로 프레임 접근
    process_image(&(buffers[buf.index]));

    // 다시 VIDIOC_QBUF로 프레임 요청
    if (xioctl(fd, VIDIOC_QBUF, &buf))
        mesg_exit("VIDIOC_QBUF");

    std::cout << "re-request frame" << "\n";

    return 1;
}

static void process_image(const void* p) {
    // something ...
    // yuyv -> h.264 코덱 -> libavformat으로 변환 후 저장

}

int initFramebuffer(ushort** fbPtr, int* size) {
    int fd = open(FB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open framebuffer device");
        return -1;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        close(fd);
        return -1;
    }

    *size = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
    *fbPtr = (ushort*)mmap(0, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*fbPtr == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        close(fd);
        return -1;
    }

    return fd;
}