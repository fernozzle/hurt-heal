typedef struct {
	unsigned int  numhurts;
	unsigned int  numheals;
	unsigned int *hurts;
	unsigned int *heals;
} votedata;

int parsevote (votedata *in);
void printvote (votedata *in);
