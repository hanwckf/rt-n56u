#  Makefile for elfls

CC = gcc
CFLAGS = -Wall -Wextra -I../elfrw

elfls: elfls.c ../elfrw/libelfrw.a

../elfrw/libelfrw.a:
	$(MAKE) -C ../elfrw libelfrw.a

clean:
	rm -f elfls
