PROG := brillo
DESC := Control the brightness of backlight and keyboard LED devices
VENDOR := com.gitlab.CameronNemo

PREFIX ?= /usr

CFLAGS += -std=c99 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror -Wextra
LDLIBS += -lm

BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1
PKEDIR=$(DESTDIR)$(PREFIX)/share/polkit-1/actions
UDEVDIR=$(DESTDIR)$(PREFIX)/lib/udev/rules.d
AADIR=$(DESTDIR)/etc/apparmor.d

SRC = \
	src/value.c \
	src/light.c \
	src/file.c \
	src/parse.c \
	src/path.c \
	src/ctrl.c \
	src/info.c \
	src/init.c \
	src/exec.c \
	src/main.c

OBJ = $(SRC:.c=.o)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

install: $(PROG)
	install -dZ -m 755 $(BINDIR)
	install -DZ -m 755 ./$(PROG) -t $(BINDIR)

GOMD2MAN := $(shell which go-md2man 2>/dev/null)

$(PROG).1:
ifndef GOMD2MAN
	$(error "go-md2man not available")
else
	$(GOMD2MAN) -in doc/man/brillo.1.md -out $@
endif

man: $(PROG).1

contrib/$(VENDOR).$(PROG).policy: contrib/polkit.in
	sed -e 's|@bindir@|$(PREFIX)/bin|g' \
	    -e 's|@prog@|$(PROG)|g' \
	    -e 's|@vendor@|$(VENDOR)|g' \
	    -e 's|@desc@|$(DESC)|g' \
	    $^ > $@

polkit: contrib/$(VENDOR).$(PROG).policy

dist: $(PROG) man polkit

install-dist: install dist
	install -dZ -m 755 $(MANDIR) $(PKEDIR) $(UDEVDIR) $(AADIR) $(AADIR)/local
	install -DZ -m 644 $(PROG).1 -t $(MANDIR)
	install -DZ -m 644 contrib/$(VENDOR).$(PROG).policy -t $(PKEDIR)
	install -DZ -m 644 contrib/90-brillo.rules -t $(UDEVDIR)
	install -DZ -m 644 contrib/bin.brillo -t $(AADIR)
	install -DZ -m 644 contrib/local/bin.brillo -t $(AADIR)/local

uninstall:
	rm -f $(BINDIR)/$(PROG)

uninstall-dist:
	rm -f $(MANDIR)/$(PROG).1
	rm -f $(PKEDIR)/$(VENDOR).$(PROG).policy
	rm -f $(UDEVDIR)/90-brillo.rules

clean:
	rm -vfr *~ $(PROG) $(PROG).1 $(OBJ) contrib/$(VENDOR).$(PROG).policy

.PHONY: install uninstall polkit dist install-dist uninstall-dist man clean
