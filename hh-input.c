#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "io.h"

#define MAXLEN 1024

int main (void) {
	char *lenstr, input[MAXLEN];
	long len;
	lenstr = getenv("CONTENT_LENGTH");
	if (lenstr == NULL) {
		printf("Content-Type: text-plain;charset=us-ascii\n\n");
		printf("No POST data recieved");
	} else {
		len = atoi(lenstr);
		fgets(input, len+1, stdin);
		
		printf("Content-Type: text/plain;charset=us-ascii\n\n");
		printf("%s", input);
	}
	return 0;
}

