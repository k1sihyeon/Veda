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


static int xioctl(int fd, int request, void* arg) {
    int r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

extern inline int clip(int value, int min, int max) {
    return (value > max ? max : value < min ? min : value);
}

static void mesg_exit(const char *s) {
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

// 이 함수로 전송하면 될 듯????
static void process_image(int csock, const void* p) {
    struct buffer* inbuff = (struct buffer*) p;

	size_t chunk = BUFSIZ;
	size_t remain = inbuff->length;
	size_t total_sent = 0;
	size_t sent;
	unsigned char* data = (unsigned char *)(inbuff->start);

	std::cout << "buf->length : " << inbuff->length << "\n";

	while (remain > 0) {
		size_t to_send = (remain < chunk) ? remain : chunk;

		std::cout << "left : " << remain << "\n";

		if ((sent = write(csock, data + total_sent, to_send)) == -1) {
			perror("Error : write()");
			break;
		}

		total_sent += sent;
		remain -= sent;
	}
    
}

static int read_frame(int csock, int fd) {
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
		switch(errno) {
			case EAGAIN: return 0;
			case EIO:
			/* Could ignore EIO, see spec. */
			default: mesg_exit("VIDIOC_DQBUF");
		}
	}

	process_image(csock, &buffers[buf.index]);

	if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	        mesg_exit("VIDIOC_QBUF");

	return 1;
}

static void mainloop(int csock, int fd) {
	//unsigned int count = 100;

	printf("mainloop entered!!!\n");

	while(true) {
		for (;;) {
			fd_set fds;
			struct timeval tv;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			int r = select(fd + 1, &fds, NULL, NULL, &tv);
			if(-1 == r) {
				if(EINTR == errno) continue;
				mesg_exit("select");
			} else if(0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}
			if(read_frame(csock, fd)) break;
		}
	}
}

static void init_mmap(int fd) {
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count       = 4;
	req.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory      = V4L2_MEMORY_MMAP;

	if(-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if(EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", VIDEODEV);
			exit(EXIT_FAILURE);
		} else {
			mesg_exit("VIDIOC_REQBUFS");
		}
	}

	if(req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", VIDEODEV);
		exit(EXIT_FAILURE);
	}

	buffers = (buffer *)calloc(req.count, sizeof(*buffers));

	if(!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
		            mesg_exit("VIDIOC_QUERYBUF");
		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
		                                        MAP_SHARED, fd, buf.m.offset);
		if(MAP_FAILED == buffers[n_buffers].start)
		            mesg_exit("mmap");
	}
}

static void init_device(int fd) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n", VIDEODEV);
            exit(EXIT_FAILURE);
        }
        else {
            mesg_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n",
                VIDEODEV);
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n", VIDEODEV);
        exit(EXIT_FAILURE);
    }

    /* Select video input, video standard and tune here. */
    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        /* reset to default */
        xioctl(fd, VIDIOC_S_CROP, &crop);
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        mesg_exit("VIDIOC_S_FMT");

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;

    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    init_mmap(fd);
}

static void start_capturing(int fd) {
    for (int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            mesg_exit("VIDIOC_QBUF");
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        mesg_exit("VIDIOC_STREAMON");
}

int main(int argc, char** argv) {
	pid_t pid;
	struct sockaddr_in servaddr, cliaddr;
	int ssock, csock, port = PORT;
	int camfd = -1;

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

	// 카메라
    camfd = open(VIDEODEV, O_RDWR | O_NONBLOCK, 0);

    if (-1 == camfd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n",
                VIDEODEV, errno, strerror(errno));
        return EXIT_FAILURE;
    }

	init_device(camfd);
    start_capturing(camfd);

	while (true) {
		//int client_port;
		socklen_t clen = sizeof(cliaddr);
		
		csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);

		printf("client connected!!\n");

		//fork
		if ((pid = fork()) < 0)
			perror("fork()");
		// in child proc.
		else if (pid == 0) {
            mainloop(csock, camfd);

            enum v4l2_buf_type type;
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == xioctl(camfd, VIDIOC_STREAMOFF, &type))
                mesg_exit("VIDIOC_STREAMOFF");

            /* 메모리 정리 */
            for (int i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length))
                    mesg_exit("munmap");
			
			free(buffers);
			close(camfd);

			exit(0);
        }
	}

	return 0;
}