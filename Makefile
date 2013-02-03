all: hurt-heal

hurt-heal: hurt-heal.c
	cc -o $@ -std=gnu99 $<

clean:
	rm -f hurt-heal
