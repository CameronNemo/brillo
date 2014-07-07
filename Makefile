PREFIX=$(DESTDIR)/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-std=c89 -O2 -pedantic -Wall -I"./include"

all:
	$(CC) $(CFLAGS) -g -o light src/helpers.c src/light.c src/main.c
exp:
	$(CC) $(CFLAGS) -E  src/helpers.c src/light.c

install: all
	mkdir -p $(BINDIR)
	cp -f ./light $(BINDIR)/light
	chown root $(BINDIR)/light
	chmod 4755 $(BINDIR)/light

uninstall:
	rm $(BINDIR)/light
	rm -rf /etc/light

clean:
	rm -vfr *~ light
	
