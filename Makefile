CC=gcc
CFLAGS=-std=c89 -pedantic -Wall

debug:clean
	$(CC) $(CFLAGS) -g -o light main.c
stable:clean
	$(CC) $(CFLAGS) -o light main.c
clean:
	rm -vfr *~ light
	