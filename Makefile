all: hurt-heal.cgi scores.cgi

hurt-heal.cgi: hurt-heal.o ip.o parsevote.o captcha.o
	cc -o $@ $^ -lcurl

hurt-heal.o: hurt-heal.c
	cc -c -o $@ $<

scores.cgi: scores.c
	cc -o $@ $<

clean:
	rm -f *.cgi *.o
