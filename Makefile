all: hurt-heal.cgi

hurt-heal.cgi: hurt-heal.o
	cc -o $@ $^

hurt-heal.o: hurt-heal.c
	cc -std=gnu99 -c -o $@ $<

clean:
	rm -f hurt-heal.cgi *.o
