#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MAXNAME 16

ssize_t readchar(int fd, char *dest) {
	static int count = 0;
	static char *bufptr, buf[BUFSIZ];
	if (count <= 0) {
		if ((count = read(fd, buf, BUFSIZ)) < 0) {
			return -1; /* error */
		} else if (count == 0) {
			return 0; /* eof */
		}
		bufptr = buf;
	}
	count--;
	*dest = *(bufptr++);
	return 1; /* success */
}

char ctoi (char c) {
	char result = c - '0';
	if (0 <= result && result <= 9) {
		return result;
	} else {
		return -1;
	}
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
	/*printf("Gonna open char info file!\n");*/
	if ((fd = open("characters", O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	unsigned int charcount;
	if (readuline(fd, &charcount) == -1) {
		perror("readchar");
		exit(1);
	}
	/*printf ("%u characters\n", charcount);*/
	
	char charnames[charcount][MAXNAME];
	unsigned int i;
	for (i = 0; i < charcount; i++) {
		readsline(fd, charnames[i], MAXNAME);
		/*printf("%2u: %s\n", i+1, charnames[i]);*/
	}
	
	/*printf("Closing char info file!\n");*/
	close(fd);

	/* scores */
	struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
	
	fl.l_pid = getpid();
	
	/*printf("Gonna open file!\n");*/
	
	if ((fd = open("scores", O_RDWR)) == -1) {
		perror("open");
		exit(1);
	}
	
	/*printf("Trying to get lock...\n");*/

	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}

	printf("Got lock!\n");
	
	unsigned int charscores[charcount];
	for (i = 0; i < charcount; i++) {
		if (readuline(fd, &(charscores[i])) == -1) {
			perror ("readchar");
			exit (1);
		}/* else {
			printf ("%2u: %5u\n", i+1, charscores[i]);
		}*/
	}
	
	printf("Press <RETURN> to release lock: ");
	getchar();
	
	fl.l_type = F_UNLCK;
	
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	
	printf("Unlocked!\n");
	
	close(fd);
	
	return 0;
}

