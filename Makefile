CC=gcc
CFLAGS=-std=c89 -O2 -pedantic -Wall -I"./include"

all:
	$(CC) $(CFLAGS) -g -o light src/helpers.c src/light.c src/main.c
exp:
	$(CC) $(CFLAGS) -E  src/helpers.c src/light.c

clean:
	rm -vfr *~ light
	
