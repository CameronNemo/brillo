PROG := brillo
DESC := Backlight and Keyboard LED control tool
VENDOR := com.gitlab.CameronNemo

ifeq ($(PREFIX),)
	PREFIX := /usr
endif

CFLAGS := -std=c99 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror $(CFLAGS)
LDLIBS := $(LDLIBS) -lm

BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1
PKEDIR=$(DESTDIR)$(PREFIX)/share/polkit-1/actions
UDEVDIR=$(DESTDIR)$(PREFIX)/lib/udev/rules.d
AADIR=$(DESTDIR)/etc/apparmor.d

GOMD2MAN := $(shell which go-md2man 2>/dev/null)

brillo: src/value.c src/light.c src/helpers.c src/parse.c src/path.c src/ctrl.c src/info.c src/init.c src/exec.c src/main.c
	$(CC) $(CFLAGS) $(LDLIBS) -o $@ $^

install: brillo
	install -dZ -m 755 $(BINDIR)
	install -DZ -m 755 ./$(PROG) -t $(BINDIR)

man:
ifndef GOMD2MAN
$(error "go-md2man not available")
endif
	$(GOMD2MAN) -in doc/man/brillo.1.md -out $(PROG).1

polkit:
	sed 's|@bindir@|$(PREFIX)/bin|g;s|@prog@|$(PROG)|g;s|@vendor@|$(VENDOR)|g;s|@desc@|$(DESC)|g' contrib/polkit.in >contrib/$(VENDOR).$(PROG).policy

dist: man polkit

install-dist: install
	install -dZ -m 755 $(MANDIR) $(PKEDIR) $(UDEVDIR) $(AADIR)
	install -DZ -m 644 $(PROG).1 -t $(MANDIR)
	install -DZ -m 644 contrib/$(VENDOR).$(PROG).policy -t $(PKEDIR)
	install -DZ -m 644 contrib/90-brillo.rules -t $(UDEVDIR)
	install -DZ -m 644 contrib/bin.brillo -t $(AADIR)

uninstall:
	rm -f $(BINDIR)/$(PROG)

uninstall-dist:
	rm -f $(MANDIR)/$(PROG).1
	rm -f $(PKEDIR)/$(VENDOR).$(PROG).policy
	rm -f $(UDEVDIR)/90-brillo.rules

clean:
	rm -vfr *~ $(PROG) $(PROG).1 contrib/$(VENDOR).$(PROG).policy

.PHONY: install uninstall clean
