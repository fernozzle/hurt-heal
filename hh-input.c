#include <stdio.h>
#include <stdlib.h>

#define MAXLEN 1024

int getnum (void) {
	unsigned int val;
	if (scanf ("%u.", &val) != 1) {
		return -1;
	} else {
		return val;
	}
}

int main (void) {
	char *lenstr, input[MAXLEN];
	long len;
	lenstr = getenv("CONTENT_LENGTH");
	if (lenstr == NULL) {
		printf("Content-Type: text-plain;charset=us-ascii\n\n");
		printf("No POST data recieved");
	} else {
		len = atoi(lenstr);

		printf("Content-Type: text/plain;charset=us-ascii\n\n");		
		unsigned int i;
		int num;
		scanf("vote=");
		unsigned int numhurts = getnum();
		unsigned int numheals = getnum();
		unsigned int *hurts = malloc(numhurts * sizeof(unsigned int));
		unsigned int *heals = malloc(numheals * sizeof(unsigned int));
		for (i = 0; i < numhurts; i++) {
			num = getnum();
			hurts[i] = num;
		}
		for (i = 0; i < numheals; i++) {
			num = getnum();
			heals[i] = num;
		}
		printf ("Hurts:\n");
		for (i = 0; i < numhurts; i++) {
			printf("%u\n", hurts[i]);
		}
		printf ("\nHeals:\n");
		for (i = 0; i < numheals; i++) {
			printf("%u\n", heals[i]);
		}
	}
	return 0;
}

