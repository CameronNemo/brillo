ifeq ($(PREFIX),)
	PREFIX := /usr
endif

BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1

CFLAGS=-std=c99 -O2 -pedantic -Wall -Werror -I"./include" -D_XOPEN_SOURCE=500
MANFLAGS=-h -h -v -V -N

HELP2MAN_VERSION := $(shell help2man --version 2>/dev/null)

light: src/helpers.c src/light.c src/main.c
	$(CC) $(CFLAGS) -g -o $@ $^

man: light
ifndef HELP2MAN_VERSION
$(error "help2man is not installed")
endif
	help2man $(MANFLAGS) ./light | gzip - > light.1.gz

install: light man
	install -dZ $(BINDIR)
	install -DZ -m 4755 ./light -t $(BINDIR)
	install -dZ $(MANDIR)
	install -DZ light.1.gz -t $(MANDIR)

uninstall:
	rm -f $(BINDIR)/light
	rm -f $(MANDIR)/light.1.gz

clean:
	rm -vfr *~ light light.1.gz

.PHONY: man install uninstall clean
