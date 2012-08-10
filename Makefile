VERSION = git-20120807

PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC      = gcc
CFLAGS  = -ansi -Wall -pedantic -O2 -I$(PREFIX)
LDFLAGS = -L$(PREFIX)
LIBS    = -lX11 -lImlib2 -lgif -lyaml

SRC = commands.c exif.c image.c options.c thumbs.c util.c window.c
OBJ = $(SRC:.c=.o)

V_SRC = main.c $(SRC)
V_OBJ = $(V_SRC:.c=.o)

T_SRC = sxit.c palette.c tag.c $(SRC)
T_OBJ = $(T_SRC:.c=.o)

P_SRC = parse_pallete.c palette.c exif.c image.c options.c thumbs.c util.c window.c
P_OBJ = $(P_SRC:.c=.o)

all: options sxiv

options:
	@echo "sxiv build options:"
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "PREFIX  = $(PREFIX)"

.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

$(OBJ): Makefile config.h

config.h:
	@echo "creating $@ from config.def.h"
	@cp config.def.h $@

sxiv:	$(OBJ)
	@echo "CC -o $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

sxit:	$(T_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(T_OBJ) $(LIBS)

pp:	$(P_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(P_OBJ) $(LIBS)

clean:
	@echo "cleaning"
	@rm -f $(OBJ) $(T_OBJ) $(V_OBJ) sxit sxiv sxiv-$(VERSION).tar.gz

install: all
	@echo "installing executable file to $(DESTDIR)$(PREFIX)/bin"
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f sxiv $(DESTDIR)$(PREFIX)/bin/
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "installing manual page to $(DESTDIR)$(MANPREFIX)/man1"
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	@sed "s/VERSION/$(VERSION)/g" sxiv.1 > $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	@chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sxiv.1

uninstall:
	@echo "removing executable file from $(DESTDIR)$(PREFIX)/bin"
	@rm -f $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "removing manual page from $(DESTDIR)$(MANPREFIX)/man1"
	@rm -f $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
