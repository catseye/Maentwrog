# GNU Makefile for maentwrog.

PROGS=bin/maentw bin/caparse bin/rpn bin/rdp
CC?=gcc

WARNS=	-W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith	-Wno-uninitialized -Wreturn-type -Wcast-qual \
	-Wwrite-strings -Wswitch -Wcast-align -Wchar-subscripts \
	-Winline -Wnested-externs -Wredundant-decls

ifdef ANSI
  CFLAGS+= -ansi -pedantic
else
  CFLAGS+= -std=c99 -D_POSIX_C_SOURCE=200112L
endif

CFLAGS+= ${WARNS} ${EXTRA_CFLAGS}

ifdef DEBUG
  CFLAGS+= -g
endif

all: $(PROGS)

bin/.exists:
	mkdir -p bin
	touch bin/.exists

bin/maentw: bin/.exists src/maentw.c
	$(CC) $(CFLAGS) src/maentw.c -o bin/maentw

bin/caparse: bin/.exists src/caparse.c
	$(CC) $(CFLAGS) src/caparse.c -o bin/caparse

bin/rpn: bin/.exists src/rpn.c
	$(CC) $(CFLAGS) src/rpn.c -lm -o bin/rpn

bin/rdp: bin/.exists src/rdp.c
	$(CC) $(CFLAGS) src/rdp.c -o bin/rdp

clean:
	rm -f *.o

distclean:
	rm -f $(PROGS)
