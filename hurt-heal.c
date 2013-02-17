#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "ip.h"

#define MAXNAMELEN 16

int getnum (void);
int applyvote (FILE *voteinput, unsigned int scores[]);

int main (void) {
	printf("Content-Type: text/plain;charset=us-ascii\n\n");
	/* read character info */
	int fd;
	FILE *fp;

	fp = fopen("characters", "r");
	if (fp == NULL) {
		perror ("Error opening character list");
		exit (EXIT_FAILURE);
	}
	unsigned int charcount;
	if (fscanf (fp, "%u\n", &charcount) != 1) {
		printf ("Error reading character count\n");
		exit (EXIT_FAILURE);
	}
	char charnames[charcount][MAXNAMELEN];
	unsigned int i;
	for (i = 0; i < charcount; i++) {
		if (fscanf (fp, "%[^\n]\n", charnames[i]) != 1) {
			printf ("Error reading character name\n");
			exit (EXIT_FAILURE);
		}
	}
	fclose(fp);
	
	/* check if this is a returning ip */
	bool canvote;
	char *ipstr;
	ipstr = getenv("REMOTE_ADDR");
	fp = fopen ("recentvoters", "a+");
	if (canvote = !checkip (fp, ipstr)) {	
		addip (fp, ipstr);
	}
	fclose(fp);
		
	/* open, lock, and read scores */
	struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, getpid()};
	if (canvote) {
		if ((fd = open("scores", O_RDWR)) == -1) {
			perror("open read/write");
			exit(1);
		}
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			perror("fcntl");
			exit(1);
		}
		fp = fdopen(fd, "r+");
	} else {
		if ((fd = open("scores", O_RDONLY)) == -1) {
			perror("open read-only");
			exit(1);
		}
		fp = fdopen(fd, "r");
	}	
	unsigned int charscores[charcount];
	for (i = 0; i < charcount; i++) {
		if (fscanf (fp, "%u\n", &(charscores[i])) != 1) {
			printf ("Error reading character score\n");
			exit (EXIT_FAILURE);
		}
	}

	if (canvote) {
		/* change scores */
		if (applyvote(stdin, charscores) == -1) {
			printf ("Unexpected vote format\n");
			exit (EXIT_FAILURE);
		}

		/* write changes, unlock, and close */
		rewind (fp);
		ftruncate (fd, 0);
		for (i = 0; i < charcount; i++) {
			fprintf (fp, "%u\n", charscores[i]);
		}
		fl.l_type = F_UNLCK;
		if (fcntl(fd, F_SETLK, &fl) == -1) {
			perror("fcntl");
			exit(EXIT_FAILURE);
		}
	}
	fclose(fp);
	
	/* make page */
	printf("Scores:\n");
	for (i = 0; i < charcount; i++) {
		printf("%20s: %u\n", charnames[i], charscores[i]);
	}
	printf ("\nIP: %s\n", ipstr);
	if (canvote) {
		printf ("You have not voted recently, so your vote has counted.\n");
	} else {
		printf ("You have voted recently, so your vote has not counted.\n");
	}
	
	return EXIT_SUCCESS;
}

int getnum (void) {
	unsigned int val;
	if (scanf ("%u.", &val) != 1) {
		return -1;
	} else {
		return val;
	}
}

int applyvote (FILE *voteinput, unsigned int scores[]) {
	unsigned int i;
	int num;
	char arg[6];
	scanf("%5s", arg);
	if (strcmp(arg, "vote=") != 0) {
		return -1;
	}
	unsigned int numhurts = getnum();
	unsigned int numheals = getnum();
	for (i = 0; i < numhurts; i++) {
		if ((num = getnum()) == -1) {
			return -1;
		} else {
			if(scores[num] > 0) {
				scores[num]--;
			}
		}
	}
	for (i = 0; i < numheals; i++) {
		if ((num = getnum()) == -1) {
			return -1;
		} else {
			scores[num]++;
		}
	}
}

