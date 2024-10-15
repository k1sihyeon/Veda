#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int fd;

int main(void) {

	//fd = open("data2.txt", O_WRONLY | O_CREAT);
	//printf("fd is: %d\n", fd);
	write(1, "hello world\n", sizeof("hello world\n"));
	//close(fd);

	return 0;
}
