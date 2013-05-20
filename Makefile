all: hurt-heal.cgi

hurt-heal.cgi: hurt-heal.o ip.o parsevote.o
	cc -o $@ $^

hurt-heal.o: hurt-heal.c
	cc -c -o $@ $<

clean:
	rm -f *.cgi *.o
