#  Makefile for sstrip

CC = gcc
CFLAGS = -Wall -Wextra -I  ./elfrw

sstrip: sstrip.c ./elfrw/libelfrw.a
./elfrw/libelfrw.a:
	$(MAKE) -C ./elfrw libelfrw.a

clean:
	for dir in elfrw ; do $(MAKE) -C$$dir clean ; done
	rm -f sstrip