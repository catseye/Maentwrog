CC?=gcc
CFLAGS=-ansi -pedantic -O

PROGS=maentw caparse rpn rdp

all: $(PROGS)

maentw: maentw.c
	$(CC) $(CFLAGS) maentw.c -o maentw

caparse: caparse.c
	$(CC) $(CFLAGS) caparse.c -o caparse

rpn: rpn.c
	$(CC) $(CFLAGS) rpn.c -lm -o rpn

rdp: rdp.c
	$(CC) $(CFLAGS) rdp.c -o rdp

clean:
	rm -f *.o $(PROGS)
