#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>
/*#include "ip.h"*/

#define PRIVKEY "6LfnpMwSAAAAAHviL1a5LBEQYai9zUdgw4FAfxtZ"
#define VERIURL "http://www.google.com/recaptcha/api/verify"

#define PRIVKEYPREFIX "privatekey="
#define REMOTEIPPREFIX "&remoteip="
#define CHALLENGEPREFIX "&challenge="
#define RESPONSEPREFIX "&response="

#define strllen(x) (sizeof x) - 1

/* a ReCaptcha success response is
----
true
success
----

If success, apply vote and echo
If failure, echo
*/

static char captchasuccess;

size_t foo (void *ptr, size_t size, size_t nmemb, void *stream) {
	/*printf ("%s\n", ptr);*/
	if (captchasuccess == -1) {
		char firstchar = ((char *)ptr)[0];
		if (firstchar == 't') {
			captchasuccess = 1;
		} else if (firstchar == 'f') {
			captchasuccess = 0;
		}
	}
	return size * nmemb;
}

bool checkcaptcha (const char * const ipstr,
                   const char * const challenge,
                   const char * const response){
	CURL *curl = curl_easy_init();
	if (curl) {
		/*char *postdata = ipstr;*/
		size_t postlength = strllen(PRIVKEYPREFIX)   + strllen(PRIVKEY) +
		                    strllen(REMOTEIPPREFIX)  + strlen (ipstr) +
		                    strllen(CHALLENGEPREFIX) + strlen (challenge) +
		                    strllen(RESPONSEPREFIX)  + strlen (response);
		char *postdata = malloc (postlength + 1);
		sprintf (postdata, "%s%s%s%s%s%s%s%s",
		         PRIVKEYPREFIX,   PRIVKEY,
		         REMOTEIPPREFIX,  ipstr,
		         CHALLENGEPREFIX, challenge,
		         RESPONSEPREFIX,  response);
		/*printf ("Post data sent to ReCaptcha: %s\n", postdata);*/
		
		curl_easy_setopt (curl, CURLOPT_POSTFIELDS, postdata);
		curl_easy_setopt (curl, CURLOPT_URL, VERIURL);
		curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, foo);
		captchasuccess = -1;
		curl_easy_perform (curl);
		curl_easy_cleanup (curl);
		/*printf ("Resulting captchasuccess: %d\n", captchasuccess);*/
		if (captchasuccess == 1) return true; /* success */
		else if (captchasuccess == 0) return false; /* incorrect (make sure to avoid timeout */
		else return false; /* some other error ('t' or 'f' not being the first thing received) */
	} else {
		return false;
	}
}
/*
int main (int argc, char** argv) {
	char *ipstr;
	ipstr = getenv("REMOTE_ADDR");
	
	printf ("Content-type: text/plain\n\n");
	checkcaptcha (ipstr, "03AHJ_VuuSd5NVNDRycDfqFMJZFZApNFwEpLUwS5BsedtelFlJ3KbNC_82rzz6Spn6xlTdAMsUKJybo8bMd7s-GlrJH2TS8L29nwyCtJdKBy0gQjzrt2ENz1RynBt0sPRJZtZyiQWwvyy7wiNh8YtQpdPBaG2d6ThJ93qRPdp45Y_e4wg5g8DwmUE"
,"miInte depression");
	return EXIT_SUCCESS;
}*/
