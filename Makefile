PROG := brillo
DESC := Control the brightness of backlight and keyboard LED devices
VENDOR := com.gitlab.CameronNemo
VERSION := 1.4.10

GOMD2MAN ?= go-md2man
GROUP ?= video

SYSCONFDIR ?= /etc
AADIR ?= $(SYSCONFDIR)/apparmor.d
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
PKEDIR ?= $(PREFIX)/share/polkit-1/actions
UDEVRULESDIR ?= $(PREFIX)/lib/udev/rules.d

ifeq ($(UBSAN),1)
override CFLAGS += \
	-fsanitize=undefined \
	-fsanitize=shift \
	-fsanitize=unreachable \
	-fsanitize=vla-bound \
	-fsanitize=null \
	-fsanitize=return \
	-fsanitize=signed-integer-overflow \
	-fsanitize=integer-divide-by-zero \
	-fsanitize=float-divide-by-zero \
	-fsanitize=float-cast-overflow \
	-fsanitize=bounds \
	-fsanitize=alignment \
	-fsanitize=object-size \
	-fsanitize=vptr
endif

override CFLAGS += \
	-std=c99 -D_XOPEN_SOURCE=700 -pedantic \
	-Wall -Werror -Wextra \
	-DPROG='"$(PROG)"' -DVERSION='"$(VERSION)"'

override LDLIBS += -lm

SRC = \
	src/vlog.c \
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

build/$(PROG): $(OBJ)
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

install.bin: build/$(PROG)
	install -Dm 0755 -t $(DESTDIR)$(BINDIR) $^

build/$(VENDOR).$(PROG): contrib/apparmor.in
	sed -e 's|@vendor@|$(VENDOR)|g' -e 's|@prog@|$(PROG)|g' $^ > $@

install.apparmor: build/$(VENDOR).$(PROG)
	install -d $(DESTDIR)$(AADIR)
	install -m 0640 -t $(DESTDIR)$(AADIR) $^

build/$(PROG).1: doc/man/brillo.1.md
	$(GOMD2MAN) -in $^ -out $@

install.man: build/$(PROG).1
	install -d $(DESTDIR)$(MANDIR)
	install -m 0644 -t $(DESTDIR)$(MANDIR) $^

build/92-$(VENDOR).$(PROG).rules: contrib/udev.in
	sed -e 's|@group@|$(GROUP)|g' $^ > $@

install.udev: build/92-$(VENDOR).$(PROG).rules
	install -d $(DESTDIR)$(UDEVRULESDIR)
	install -m 00644 -t $(DESTDIR)$(UDEVRULESDIR) $^

install.common: install.man install.udev

install: install.bin install.common

install.bin.setgid: build/$(PROG)
	install -d $(DESTDIR)$(BINDIR)
	install -m 2755 -g $(GROUP) -t $(DESTDIR)$(BINDIR) $^

install.setgid: install.bin.setgid install.common

build/$(VENDOR).$(PROG).policy: contrib/polkit.in
	sed -e 's|@bindir@|$(BINDIR)|g' \
	    -e 's|@prog@|$(PROG)|g' \
	    -e 's|@vendor@|$(VENDOR)|g' \
	    -e 's|@desc@|$(DESC)|g' \
	    $^ > $@

install.polkit: build/$(VENDOR).$(PROG).policy
	install -d $(DESTDIR)$(PKEDIR)
	install -m 0644 -t $(DESTDIR)$(PKEDIR) $^

dist:

install-dist: install.bin install.common install.polkit

clean:
	rm -rfv -- *~ $(OBJ) build

.PHONY: install.bin install.apparmor install.man install.udev install.common install install.setgid install.polkit dist install-dist clean
