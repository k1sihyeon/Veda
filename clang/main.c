#include <stdio.h>
#include <string.h>
#include "copy.h"

#define MAX_LINE 100

char line[MAX_LINE];
char longest[MAX_LINE];

int main(void) {

int len, max = 0;

while (gets(line) != NULL) {

len = strlen(line);

	if (len > max) {
		max = len;
		copy(line, longest);
	}
}

if (max > 0)
	printf("%s", longest);

return 0;
}
