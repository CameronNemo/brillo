ifeq ($(PREFIX),)
	PREFIX := /usr
endif

BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1
PKEDIR=$(DESTDIR)$(PREFIX)/share/polkit-1/actions
UDEVDIR=$(DESTDIR)$(PREFIX)/lib/udev/rules.d
AADIR=$(DESTDIR)/etc/apparmor.d

CFLAGS=-std=c99 -O2 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700
MANFLAGS=-h -h -v -V -N

HELP2MAN_VERSION := $(shell help2man --version 2>/dev/null)

light: src/helpers.c src/light.c src/light_init.c src/main.c
	$(CC) $(CFLAGS) -g -o $@ $^

man: light
ifndef HELP2MAN_VERSION
$(error "help2man is not installed")
endif
	help2man $(MANFLAGS) ./light | gzip - > light.1.gz

polkit:
	sed 's|@bindir@|$(BINDIR)|g' contrib/light.policy.in >contrib/light.policy

install: man polkit
	install -dZ $(BINDIR) $(MANDIR) $(PKEDIR) $(UDEVDIR) $(AADIR)
	install -DZ -m 755 ./light -t $(BINDIR)
	install -DZ -m 644 light.1.gz -t $(MANDIR)
	install -DZ -m 644 contrib/light.policy -t $(PKEDIR)
	install -DZ -m 644 contrib/90-backlight.rules -t $(UDEVDIR)
	install -DZ -m 644 contrib/usr.bin.light -t $(AADIR)

uninstall:
	rm -f $(BINDIR)/light
	rm -f $(MANDIR)/light.1.gz
	rm -f $(PKEDIR)/light.policy
	rm -f $(UDEVDIR)/90-backlight.rules

clean:
	rm -vfr *~ light light.1.gz contrib/light.policy

.PHONY: man install uninstall clean polkit
