#include <stdlib.h>

int randrange (int min, int max) {
	return min + (rand() % (int)(max - min + 1));
}

void randalphanum (char *dest, unsigned int length) {
	srand(time(NULL));
	unsigned int i;	
	for (i = 0; i < length; i++) {
		unsigned randchar = randrange (0, 26+26+10-1);
		if (randchar < 26) {
			dest[i] = 'a' + randchar;
		} else if (randchar < 26+26) {
			dest[i] = 'A' + randchar - 26;
		} else {
			dest[i] = '0' + randchar - 26 - 26;
		}
	}
	dest[length] = 0;
}
