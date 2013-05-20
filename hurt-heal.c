#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "parsevote.h"
#include "ip.h"

#define MAXNAMELEN 16
#define CHARFILE "characters"
#define SCOREFILE "scores"
#define HISTORYFILE "scorehistory"
#define RECENTVOTERS "recentvoters"
#define BLACKLIST "blacklist"

#define QUALBLKLIST 1 << 0
#define QUALRECENTV 1 << 1

#define DATELEN 9

int applyvote (votedata *vote, unsigned int scores[]);
void printscores    (const char *filename);
void printscorehistory (const char *filename);
void printcharnames (const char *filename);
unsigned int getcharcount (const char *filename) {
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror ("Error opening character list");
		exit (EXIT_FAILURE);
	}
	unsigned int charcount;
	if (fscanf (fp, "%u\n", &charcount) != 1) {
		printf ("Error reading character count from list\n");
		exit (EXIT_FAILURE);
	}
	fclose (fp);
	return charcount;
}
void datestring (char *ds) {
	time_t currenttime;
	struct tm *tmtime;
	
	currenttime = time (NULL);
	tmtime = localtime (&currenttime);
	if (strftime (ds, DATELEN, "%Y%m%d", tmtime) != 8) {
		fprintf (stderr, "Error in making time string\n");
		exit (EXIT_FAILURE);
	}
}

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
}

int main (void) {
	
	printf("Content-Type: application/javascript;charset=utf-8\n");
	printf("\n");
	
	char *methodstr;
	if ((methodstr = getenv ("REQUEST_METHOD")) == NULL) {
		fprintf (stderr, "Not CGI\n");
		exit (EXIT_FAILURE);
	}
		
	if (methodstr[0] == 'P') {
		/* read vote */
		votedata *invote;
		invote = malloc (sizeof(votedata));
		if (parsevote(invote) == -1) {
			printf ("Unexpected vote format\n");
			exit (EXIT_FAILURE);
		}
	
		/* read character info */
		unsigned int charcount = getcharcount (CHARFILE);
	
		char *ipstr;
		if ((ipstr = getenv("REMOTE_ADDR")) == NULL) {
			fprintf (stderr, "Not CGI\n");
		}
		unsigned int unable = 0;
		qualifications (ipstr, invote, &unable);
	
		/* open, lock, and read scores */
		if (!unable) {
			int fd;
			if ((fd = open(SCOREFILE, O_RDWR)) == -1) {
				perror("open read/write");
				exit(1);
			}
			struct flock fl;
			fl = (struct flock) {F_WRLCK, SEEK_SET, 0, 0, getpid()};
			if (fcntl(fd, F_SETLKW, &fl) == -1) {
				perror("fcntl");
				exit(1);
			}
			FILE *fp;
			fp = fdopen(fd, "r+");
			fscanf (fp, "%*u\n"); /* ignore date */
			
			unsigned int charscores[charcount], i;
			for (i = 0; i < charcount; i++) {
				if (fscanf (fp, "%u\n", &(charscores[i])) != 1) {
					printf ("Error reading character score\n");
					exit (EXIT_FAILURE);
				}
			}
			/* change scores */
			applyvote(invote, charscores);
		
			/* write changes, unlock, and close */
			rewind (fp);
			ftruncate (fd, 0);
			char *ds;
			ds = malloc (DATELEN * sizeof (char));
			datestring (ds);
			fprintf (fp, "%s\n", ds);
			for (i = 0; i < charcount; i++) {
				fprintf (fp, "%u\n", charscores[i]);
			}
			fl.l_type = F_UNLCK;
			if (fcntl(fd, F_SETLK, &fl) == -1) {
				perror("fcntl");
				exit(EXIT_FAILURE);
			}
			fclose(fp);
		}
		
		if (!unable) printf ("v(true,0)");
		else         printf ("v(false,%d)", unable);
	} else {
		char *query;
		if ((query = getenv ("QUERY_STRING")) == NULL) {
			printf ("HI!!! :) XD YOLO\n");
			fprintf (stderr, "Error getting query string\n");
			exit (EXIT_FAILURE);
		} else {
			switch (query[0]) {
				case 's':
					printscores (SCOREFILE);
					break;
				case 'h':
					printscorehistory (HISTORYFILE);
					break;
				case 'c':
					printcharnames (CHARFILE);
					break;
				default:
					;
			}
		}
	}
	return EXIT_SUCCESS;
}
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
int printset (FILE *fp, unsigned int charcount) {
	char *ds;
	ds = malloc (sizeof (char) * DATELEN);
	if (fscanf (fp, "%s\n", ds) != 1) {
		return -1;
	}
	printf ("{'d':'%s','s':[", ds);
	
	bool prev = false;
	unsigned int score;
	while (charcount-- && fscanf (fp, "%u\n", &score) == 1) {
		if (prev) printf (", ");		
		printf ("%u", score);
		prev = true;;
	}
	printf ("]}");
	char separator[6];
	if (fscanf (fp, "%s\n", separator) == 1) {
		return 1;
	} else {
		return 0;
	}
}
void printscores (const char *filename) {
	int fd;
	if ((fd = open (filename, O_RDONLY)) == -1) {
		perror("opening score file");
		exit (EXIT_FAILURE);
	}
	struct flock fl;
	fl = (struct flock) {F_RDLCK, SEEK_SET, 0, 0, getpid()};
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	FILE *fp;
	fp = fdopen (fd, "r");
	
	printf ("s(");
	unsigned int charcount = getcharcount (CHARFILE);
	printset (fp, charcount);
	fclose (fp);
	printf (");");
}
void printscorehistory (const char *filename) {
	int fd;
	if ((fd = open (filename, O_RDONLY)) == -1) {
		perror("opening score file");
		exit (EXIT_FAILURE);
	}
	struct flock fl;
	fl = (struct flock) {F_RDLCK, SEEK_SET, 0, 0, getpid()};
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	FILE *fp;
	fp = fdopen (fd, "r");
	
	unsigned int charcount = getcharcount (CHARFILE);
	
	printf ("h([");
	while (printset (fp, charcount) == 1) {
		printf (", ");
	}
	fclose (fp);
	printf ("]);");
}
void printcharnames (const char *filename) {
	FILE *fp;
	if ((fp = fopen (filename, "r")) == NULL) {
		perror ("opening character info file");
		exit (EXIT_FAILURE);
	}
	
	printf ("c([");
	char c;
	bool start = false;
	fscanf (fp, "%*d\n");
	printf ("'");
	while ((c = fgetc (fp)) != EOF) {
		if (c == '\n') {
			start = true;
		} else {
			if (start) {
				printf ("','");
			}
			putc (c, stdout);
			start = false;
		}
	}
	printf ("'");
	fclose (fp);
	
	printf ("]);");
}
