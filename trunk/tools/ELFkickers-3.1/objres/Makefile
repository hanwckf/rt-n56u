#  Makefile for objres

CC = gcc
CFLAGS = -Wall -Wextra -I../elfrw
LDFLAGS = -Wall -Wextra

objres: objres.o ../elfrw/libelfrw.a

objres.o: objres.c ../elfrw/elfrw.h

../elfrw/libelfrw.a:
	$(MAKE) -C ../elfrw libelfrw.a

clean:
	rm -f objres objres.o
