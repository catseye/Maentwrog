# GNU Makefile for maentwrog.

PROGS=bin/maentw bin/caparse bin/rpn bin/rdp
CC?=gcc
RM_F?=rm -f

WARNS=	-W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith	-Wno-uninitialized -Wreturn-type -Wcast-qual \
	-Wwrite-strings -Wswitch -Wcast-align -Wchar-subscripts \
	-Winline -Wnested-externs -Wredundant-decls

ifdef ANSI
  CFLAGS+= -ansi -pedantic -D_BSD_SOURCE
else
  CFLAGS+= -std=c99 -D_POSIX_C_SOURCE=200809L
endif

CFLAGS+= ${WARNS} ${EXTRA_CFLAGS}

ifdef DEBUG
  CFLAGS+= -g
endif

all: $(PROGS)

bin/maentw: src/maentw.c
	$(CC) $(CFLAGS) src/maentw.c -o bin/maentw

bin/caparse: src/caparse.c
	$(CC) $(CFLAGS) src/caparse.c -o bin/caparse

bin/rpn: src/rpn.c
	$(CC) $(CFLAGS) src/rpn.c -lm -o bin/rpn

bin/rdp: src/rdp.c
	$(CC) $(CFLAGS) src/rdp.c -o bin/rdp

clean:
	$(RM_F) src/*.o

distclean:
	$(RM_F) $(PROGS)
