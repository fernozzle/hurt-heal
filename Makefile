all: hurt-heal.cgi

hurt-heal.cgi: hurt-heal.o ip.o parsevote.o captcha.o
	cc -o $@ $^ -lcurl

hurt-heal.o: hurt-heal.c
	cc -std=gnu99 -c -o $@ $<

clean:
	rm -f *.cgi *.o
