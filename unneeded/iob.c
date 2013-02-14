#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define BUFSIZE 5

typedef struct {
	char    buf[BUFSIZE];
	char   *bufptr;
	ssize_t count;
	int     fd;
} iob;

ssize_t readchar (iob *input, char *dest) {	
	if (input->count <= 0) { /* buffer empty */
		if ((input->count = read(input->fd, input->buf, BUFSIZE)) < 0) {
			return -1; /* error */
		} else if (input->count == 0) {
			return 0; /* eof */
		}
		input->bufptr = input->buf;
	}
	*dest = *(input->bufptr++);
	input->count--;
	return 1; /* success */
}

*iob 

int main (void) {
	iob *test;
	&test = iobopen("testy", O_RDONLY);
}
