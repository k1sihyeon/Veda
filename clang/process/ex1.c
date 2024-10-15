#include <stdio.h>

int main(void) {

	while (1) {
		printf(".");
		sleep(5);
		fflush(stdout);
	}

	return 0;
}
