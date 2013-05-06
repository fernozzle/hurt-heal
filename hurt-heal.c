#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "parsevote.h"
#include "ip.h"
#include "captcha.h"

#define MAXNAMELEN 16
#define CHARFILE "characters"
#define SCOREFILE "scores"
#define RECENTVOTERS "recentvoters"
#define BLACKLIST "blacklist"

#define QUALBLKLIST 1 << 0
#define QUALRECENTV 1 << 1
#define QUALFAILCAP 1 << 2

/*int getnum (void);
int applyvote (FILE *voteinput, unsigned int scores[]);*/
int applyvote (votedata *vote, unsigned int scores[]);

ssize_t qualifications (const char *ipstr,
			const votedata *invote,
                        unsigned int *unable) {
	/* check if this ip is a returning ip */
	FILE *fp = fopen (BLACKLIST, "r");
	if (fp == NULL) {
		perror ("opening blacklist");
		return -1;
	}
	if (checkip (fp, ipstr)) *unable |= QUALBLKLIST;
	fclose (fp);
	if ((fp = fopen (RECENTVOTERS, "a+")) == NULL) {
		perror ("opening list of recent voters");
		return -1;
	}
	if (checkip (fp, ipstr)) *unable |= QUALRECENTV;
	else addip (fp, ipstr);
	fclose (fp);
	/* check the captcha */
	if (!checkcaptcha (ipstr, invote->challenge, invote->solution)) {
		*unable |= QUALFAILCAP;
	}
}

int main (void) {
	
	printf("Content-Type: text/plain;charset=utf-8\n");
	printf("\n");

	/* read vote */
	votedata *invote;
	invote = malloc (sizeof(votedata));
	if (parsevote(invote) == -1) {
		printf ("Unexpected vote format\n");
		exit (EXIT_FAILURE);
	}
	
	/* read character info */
	unsigned int charcount;
	/*char charnames[charcount][MAXNAMELEN];*/
	char **charnames;
	int fd;
	FILE *fp;

	fp = fopen(CHARFILE, "r");
	if (fp == NULL) {
		perror ("Error opening character list");
		exit (EXIT_FAILURE);
	}
	if (fscanf (fp, "%u\n", &charcount) != 1) {
		printf ("Error reading character count\n");
		exit (EXIT_FAILURE);
	}
	unsigned int i;
	/*charnames = malloc (charcount * sizeof (char *));
	unsigned int i;
	for (i = 0; i < charcount; i++) {
		if (fscanf (fp, "%m[^\n]\n", &charnames[i]) != 1) {
			printf ("Error reading character name %d\n", i);
			exit (EXIT_FAILURE);
		}
	}*/
	fclose(fp);
	
	char *ipstr;
	if ((ipstr = getenv("REMOTE_ADDR")) == NULL) {
		fprintf (stderr, "Not CGI\n");
	}
	unsigned int unable = 0;
	qualifications (ipstr, invote, &unable);
	
	/* open, lock, and read scores */
	struct flock fl;
	if (!unable) {
		if ((fd = open(SCOREFILE, O_RDWR)) == -1) {
			perror("open read/write");
			exit(1);
		}
		fl = (struct flock) {F_WRLCK, SEEK_SET, 0, 0, getpid()};
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			perror("fcntl");
			exit(1);
		}
		fp = fdopen(fd, "r+");
	} else {
		if ((fd = open(SCOREFILE, O_RDONLY)) == -1) {
			perror("open read-only");
			exit(1);
		}
		fl = (struct flock) {F_RDLCK, SEEK_SET, 0, 0, getpid()};
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			perror("fcntl");
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
	
	if (!unable) {
		/* change scores */
		applyvote(invote, charscores);
		
		/* write changes, unlock, and close */
		rewind (fp);
		ftruncate (fd, 0);
		for (i = 0; i < charcount; i++) {
			fprintf (fp, "%u\n", charscores[i]);
		}
	}
	fl.l_type = F_UNLCK;
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		exit(EXIT_FAILURE);
	}
	fclose(fp);
	
	/* make page */
	/*if (canvote) {*/
		printvote (invote);
	/*}*/
	printf("Scores:\n");
	for (i = 0; i < charcount; i++) {
		/*printf("%20s: %u\n", charnames[i], charscores[i]);*/
		printf ("Character %2u: %u\n", i, charscores[i]);
	}
	printf ("\nIP address: %s\n", ipstr);
	if (!unable) {
		printf ("Your vote has been cast.\n");
	} else {
		if (unable & QUALFAILCAP) {
			printf ("You may have mistyped the ReCaptcha.\n");
		}
		if (unable & QUALRECENTV) {
			printf ("You have voted recently, so your vote has not counted.\n");
		}
		if (unable & QUALBLKLIST) {
			printf ("Your IP address is on the blacklist.");
		}
	}
	
	return EXIT_SUCCESS;
}
/*
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
*/
int applyvote (votedata *vote, unsigned int scores[]) {
	unsigned int i, charindex;
	for (i = 0; i < vote->numhurts; i++) {
		charindex = vote->hurts[i];
		if (scores[charindex] > 0) scores[charindex]--;
	}
	for (i = 0; i < vote->numheals; i++) {
		charindex = vote->heals[i];
		if (true) scores[charindex]++;
	}
}
