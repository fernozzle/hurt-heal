#include <stdio.h>
#include "rando.h"

#define VOTERFILE "voter.html"
#define RIDFILE "rids"

int main (void) {
	char rid[33];
	randalphanum (rid, 32);
	printf ("Content-Type: text/html; charset=us-ascii\n");
	printf("Set-Cookie: google_ad_key=%s; Path=/; HttpOnly\n", rid);
	printf("\n");
	
	FILE *fp;
	fp = fopen (VOTERFILE, "r");
	char c;	
	while ((c = fgetc(fp)) != EOF) {
		fputc (c, stdout);
	}
	fclose(fp);
	
	fp = fopen (RIDFILE, "a");
	fprintf (fp, "%s\n", rid);
	fclose (fp);

	return 0;
}
