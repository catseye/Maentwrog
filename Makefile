CC?=gcc
CFLAGS=-ansi -pedantic -O

PROGS=bin/maentw bin/caparse bin/rpn bin/rdp

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
