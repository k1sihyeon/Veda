#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int kbhit(void) {

struct termios oldt, newt;
int ch, oldf;

tcgetattr(0, &oldt);
newt = oldt;
newt.c_lflag &= ~(ICANON | ECHO);
oldf = fcntl(0, TCSANOW, &newt);
fcntl(0, F_SETFL, oldf | O_NONBLOCK);
ch = getchar();

tcsetattr(0, TCSANOW, &oldt);
fcntl(0, F_SETFL, oldf);

if (ch != EOF) {
	ungetc(ch, stdin);
	return 1;
}

return 0;

}


int main(int argc, char** argv) {

int i = 0;

for (i = 0; ; i++) {
if (kbhit()) {
	switch(getchar()) {
		case 'q':
		goto END;
		break;
	};
}

printf("%20d\t\t\r", i);
usleep(100);

}

END:
printf("good bye!\n");
return 0;
}
