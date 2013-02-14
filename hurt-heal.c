#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#define MAXNAMELEN 16


int main (void) {
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

	/* open, lock, and read scores */
	struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, getpid()};
	if ((fd = open("scores", O_RDWR)) == -1) {
		perror("open");
		exit(1);
	}
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(1);
	}
	fp = fdopen(fd, "r+");
	unsigned int charscores[charcount];
	for (i = 0; i < charcount; i++) {
		if (fscanf (fp, "%u\n", &(charscores[i])) != 1) {
			printf ("Error reading character score\n");
			exit (EXIT_FAILURE);
		}
	}

	/* change scores */
	unsigned static int hurtcount = 3;
	unsigned static int healcount = 2;
	unsigned int hurtlist[] = {2, 7, 15};
	unsigned int heallist[] = {3, 30};
	for(i = 0; i < hurtcount; i++) {
		charscores[hurtlist[i]]--;
	}
	for(i = 0; i < healcount; i++) {
		charscores[heallist[i]]++;
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
		exit(1);
	}
	fclose(fp);
	
	return EXIT_SUCCESS;
}

