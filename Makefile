all: hurt-heal

hurt-heal: foo.c
	cc -o hurt-heal -std=c99 hurt-heal.c

clean:
	rm -f hurt-heal
