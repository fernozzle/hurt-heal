#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "ip.h"

#define CHALLEN 256
#define CHALURL "http://www.google.com/recaptcha/api/challenge?k=6LfnpMwSAAAAAHtjh-GmvaHUQNexX6CaqWPId8Ks"

int main (void) {
	printf("Content-Type: text-plain;charset=us-ascii\n\n");
	
	char *ipstr;
	ipstr = getenv("REMOTE_ADDR");
	FILE *votedfile = fopen ("recentvoters", "r");
	bool voted = checkip (votedfile, ipstr);
	fclose (votedfile);
	
	if (voted) {
		printf ("A vote from your IP address has already been cast.\n");
		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt (curl, CURLOPT_URL, CHALURL);
			res = curl_easy_perform (curl);
			if (res != CURLE_OK) {
				printf ("Problem getting ReCaptcha challenge\n");
			}
			curl_easy_cleanup (curl);
		}
	} else {
		printf ("You're okay\n");
	}
	printf ("IP address: %s\n", ipstr);
	return 0;
}
