#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define MAXNAME 16

ssize_t readchar(int fd, char *dest) {
	static int count = 0;
	static char *bufptr, buf[BUFSIZ];
	if (count <= 0) { /* buffer empty, refill */
		if ((count = read(fd, buf, BUFSIZ)) < 0) {
			return -1; /* error */
		} else if (count == 0) {
			return 0; /* eof */
		}
		bufptr = buf;
	}
	*dest = *(bufptr++);
	count--;
	return 1; /* success */
}

ssize_t writechar(int fd, char src, bool flush) {
	static int count = 0;
	static char buf[BUFSIZ];
	buf[count] = src;
	count++;
	if (count >= BUFSIZ || flush) { /* buffer full, flush it out */
		if (write(fd, buf, count) < 0) {
			return -1; /* error */
		}
		count = 0;
	}
	return 1;
}

char ctoi (char c) {
	char result = c - '0';
	if (0 <= result && result <= 9) {
		return result;
	} else {
		return -1;
	}
}

char itoc (char i) {
	if (i >= 0 && i <= 9) {
		return i + '0';
	} else {
		return -1;
	}
}

unsigned int digitcount (unsigned int x) {
	if (x == 0) return 1;	
	int count = 0, test = 1;
	while (test <= x) {
		test *= 10;
		count++;
	}
	return count;
}

int upow (int base, unsigned int power) {
	unsigned int i;
	int value = 1;
	for (i = 0; i < power; i++) {
		value *= base;
	}
	return value;
}

char digit (unsigned int x, unsigned int index) {
	return (x / upow(10, index)) % 10;
}

ssize_t writeuline (int fd, unsigned int src) {
	unsigned int digits = digitcount(src), i;
	char currentdigit;
	for (i = 0; i < digits; i++) {
		currentdigit = digit(src, digits - i - 1);
		writechar(fd, itoc(currentdigit), false);
	}
	writechar(fd, '\n', false);
}

ssize_t readuline (int fd, unsigned int *dest) {
	int rc, totalval = 0, digits = 0;
	char c, currentdigit = 0;
	while (1) {
		if ((rc = readchar(fd, &c)) == 1) {
			if ((currentdigit = ctoi(c)) != -1) {
				totalval *= 10;
				totalval += currentdigit;
				digits++;
			} else { /* not a digit */
				if (c == '\n') {
					break;
				}
			}
		} else if (rc == 0) { /* EOF */
			break; /* currently no check for no int found */
		} else { /* error */
			return -1;
		}
	}
	*dest = totalval;
	return 1;
}

ssize_t readsline (int fd, void *buf, size_t maxlen) {
	int i, rc;
	char c, *bufptr = buf;
	
	for (i = 0; i < maxlen - 1; i++) {
		if ((rc = readchar(fd, &c)) == 1) {
			if (c == '\n') {
				break;
			}
			*bufptr++ = c;
		} else if (rc == 0) {
			if (i == 0) {
				return 0; /* EOF */
			} else {
				break;
			}
		} else {
			return -1;
		}
	}
	*bufptr = 0;
	return i + 1;
}

int main (void) {
	/* character info */
	int fd;
	if ((fd = open("characters", O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	unsigned int charcount;
	if (readuline(fd, &charcount) == -1) {
		perror("readchar");
		exit(1);
	}
	char charnames[charcount][MAXNAME];
	unsigned int i;
	for (i = 0; i < charcount; i++) {
		readsline(fd, charnames[i], MAXNAME);
	}
	close(fd);

	/* scores */
	struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
	
	fl.l_pid = getpid();
	if ((fd = open("scores", O_RDWR)) == -1) {
		perror("open");
		exit(1);
	}
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	/*printf("Got lock!\n");*/
	
	unsigned int charscores[charcount];
	for (i = 0; i < charcount; i++) {
		if (readuline(fd, &(charscores[i])) == -1) {
			perror ("readchar");
			exit (1);
		}
	}
	for(i = 0; i < charcount; i++) {
		charscores[i]++;
	}
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, 0);
	for (i = 0; i < charcount; i++) {
		writeuline(fd, charscores[i]);
	}
	writechar(fd, '\n', true);
	
	/*printf("Press <RETURN> to release lock: ");
	getchar();*/
	
	fl.l_type = F_UNLCK;
	
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	
	/*printf("Unlocked!\n");*/
	
	close(fd);
	
	return 0;
}

