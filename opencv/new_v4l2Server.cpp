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

#define PORT 	    5100
#define MAX_CLIENT  10

struct buffer {
    void* start;
    size_t length;
};

static int csock;

struct buffer *buffers = NULL
static unsigned int n_buffers = 0;  // 한번에 받아온 (req.size) 버퍼 개수
static struct fb_var_screeninfo vinfo;

static int xioctl(int fd, int request, void* arg);

extern inline int clip(int value, int min, int max) {
    return (value > max ? max : value < min ? min : value);
}

static void mesg_exit(const char *s) {
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int read_frame(int fd);
static void send_image(const void* p);

int main(int argc, char** argv) {

    pid_t pid;
    struct sockaddr_in servaddr, cliaddr;
    int ssock, port = PORT;
    int camfd = -1;

    /////////////////////////////////
    // 서버 등록
    if (argc == 2)
		port = atoi(argv[1]);
	
	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return EXIT_FAILURE;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind()");
		return EXIT_FAILURE;
	}

	if (listen(ssock, MAX_CLIENT) < 0) {
		perror("listen()");
		return EXIT_FAILURE;
	}

    ///////////////////////////
    // initdevice
	// 카메라 장치 열기
    camfd = open(VIDEODEV, O_RDWR | O_NONBLOCK, 0);

    if (-1 == camfd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", VIDEODEV, errno, strerror(errno));
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
            fprintf(stderr, "%s is no V4L2 device\n", VIDEODEV);
            exit(EXIT_FAILURE);
        }
        else
            mesg_exit("VIDEO_QUERYCAP");
    }

    // 캡쳐 불가능 한 경우 에러 처리
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", VIDEODEV);
        exit(EXIT_FAILURE);
    }

    // 스트리밍 불가능 한 경우 에러 처리
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n", VIDEODEV);
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

    if (xioctl(camfd, VIDIOC_S_FMT, &fmt) == -1) 
        mesg_exit("VIDEO_S_FMT");

    // 아마 드라이버 오류 처리??
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;
    
    //////////////////////////////
    // 버퍼 요청 - init_mmap
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count   = 4;              // 4?
    req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory  = V4L2_MEMORY_MMAP;

    // 버퍼 요청 - VIDIOC_REQBUFS
    if (xioctl(camfd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINCAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", VIDEODEV);
			exit(EXIT_FAILURE);
        }
        else
            mesg_exit("VIDIOC_REQBUFS");
    }

    // 요청 오류 처리
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", VIDEODEV);
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
        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cmafd, buf.m.offset);

        // 배열 삽입 오류 처리
        if (buffers[n_buffers].start == MAP_FAILED)
            mesg_exit("mmap");
    }
    /////////////////////////////
    // accept client
    printf("cam settings done!\n");

    char mesg[BUFSIZ];
    socklen_t clen = sizeof(cliaddr);	
	
    csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
    inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
    printf("Client is connected : %s\n", mesg);

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

    // 스트림 켜기
    if (xioctl(camfd, VIDIOC_STREAMON, &type))
        mesg_exit("VIDIOC_STREAMON");
    
    /////////////////////
    // mainloop - polling wait
    while(true) {
        while(true) {
            fd_set fds;
            struct timval tv;
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

    // index로 프레임 접근
    send_image(buffers[buf.index]);

    // 다시 VIDIOC_QBUF로 프레임 요청
    if (xioctl(fd, VIDIOC_QBUF, &buf))
        mesg_exit("VIDIOC_QBUF");

    return 1;
}

static void send_image(const void* p) {
    struct buffer* inbuff = (struct buffer*) p;

	//size_t chunk = BUFSIZ;
	size_t remain = inbuff->length;
	size_t total_sent = 0;
	size_t sent = 0;
	unsigned char* data = (unsigned char *)(inbuff->start);

	std::cout << "buf->length : " << inbuff->length << "\n";

	while (remain > 0) {
		size_t to_send = inbuff->length - sent;

		//std::cout << "left : " << remain << "\n";

		if ((sent = write(csock, data + total_sent, to_send)) <= 0) {
			perror("Error : write()");
			break;
		}

		total_sent += sent;
		remain -= sent;
	}
}