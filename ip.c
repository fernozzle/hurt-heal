#include <stdio.h>
#include <stdbool.h>

#define MAX_IP_LEN 40

/* assumes file mode is a+ */

int addip (FILE *listfile, const char *ipstr) {
	fprintf(listfile, "%s\n", ipstr);
}
bool checkip (FILE *listfile, const char *ipstr) {
	char currentipstr [MAX_IP_LEN];
	rewind (listfile);
	while (fscanf(listfile, "%s\n", currentipstr) == 1) {
		if (strcmp (currentipstr, ipstr) == 0) {
			return true;
		}
	}
	return false;
}
