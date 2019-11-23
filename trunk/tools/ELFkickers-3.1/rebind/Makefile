#  Makefile for rebind

CC = gcc
CFLAGS = -Wall -Wextra -I../elfrw

rebind: rebind.c ../elfrw/libelfrw.a

../elfrw/libelfrw.a:
	$(MAKE) -C ../elfrw libelfrw.a

clean:
	rm -f rebind
