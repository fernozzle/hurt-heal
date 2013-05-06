#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "parsevote.h"
/*#include "ip.h"*/

#define PRESTRING "v="

#define CHALLEN 256
#define SOLULEN 64

int parsevote (votedata *in);
void printvote (votedata *in);
int getnum (void);
/*
int main (void) {
	char *lenstr;
	lenstr = getenv("CONTENT_LENGTH");
	if (lenstr == NULL) {
		printf("Content-Type: text-plain;charset=us-ascii\n\n");
		printf("No POST data recieved");
	} else {
		printf("Content-Type: text-plain;charset=us-ascii\n\n");
		votedata vote;
		if (parsevote (&vote) != -1) {
			printvote (&vote);
		} else {
			printf ("problem");
		}
	}
	return EXIT_SUCCESS;
}
*/
int verifyvote (votedata *v) {
	return 0;
}

int parsevote (votedata *in) {
	/* vote format: v=[numhurts].[numheals].[hurt].[heal].[challenge].[solution]_ */
	/* challenge is base64 (with '-' and '_'), solution is only *unlikely* to have '_' */
	unsigned int i;
	/*char prefix[3];
	if (scanf ("%2s", prefix) != 1) return -1;
	if (strcmp(prefix, PRESTRING) != 0) return -1;*/
	
	if ((in->numhurts = getnum()) == -1) return -1;
	if ((in->numheals = getnum()) == -1) return -1;

	in->hurts = malloc (in->numhurts * sizeof (unsigned int));
	for (i = 0; i < in->numhurts; i++) {
		if ((in->hurts [i] = getnum()) == -1) return -1;
	}
	
	in->heals = malloc (in->numheals * sizeof (unsigned int));
	for (i = 0; i < in->numheals; i++) {
		if ((in->heals [i] = getnum()) == -1) return -1;
	}
	
	in->challenge = malloc (CHALLEN * sizeof (char));
	if (scanf ("%[^.].", in->challenge) != 1) return -1;
	
	in->solution = malloc (SOLULEN * sizeof (char));
	if (scanf ("%[^_]", in->solution) != 1) return -1;
	
	return 0;
}

void printvote (votedata *in) {
	unsigned int i;	
	printf ("numhurts:  %i\n", in->numhurts);
	printf ("  hurts: ");
	for (i = 0; i < in->numhurts; i++) printf ("%i ", in->hurts[i]);
	printf ("\n");
	printf ("numheals:  %i\n", in->numheals);
	printf ("  hurts: ");
	for (i = 0; i < in->numheals; i++) printf ("%i ", in->heals[i]);
	printf ("\n");
	printf ("challenge: %s\n", in->challenge);
	printf ("solution:  %s\n", in->solution);
}

int getnum (void) {
	unsigned int val;
	if (scanf ("%u.", &val) != 1) {
		return -1;
	} else {
		return val;
	}
}
