#  Makefile for elftoc

CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -Wall -Wextra

OBJS = gen.o names.o address.o readelf.o shdrtab.o phdrtab.o dynamic.o \
       pieces.o outbase.o outitems.o outelf64.o outelf32.o out.o elftoc.o

all: elftoc

elftoc: $(OBJS)

gen.o     : gen.c gen.h
names.o   : names.c names.h gen.h elfnames.c
address.o : address.c address.h gen.h outbase.h outitems.h
readelf.o : readelf.c readelf.h gen.h names.h pieces.h phdrtab.h shdrtab.h
phdrtab.o : phdrtab.c phdrtab.h gen.h readelf.h pieces.h address.h dynamic.h
shdrtab.o : shdrtab.c shdrtab.h gen.h readelf.h names.h pieces.h \
            outbase.h outitems.h
dynamic.o : dynamic.c dynamic.h gen.h pieces.h address.h readelf.h outitems.h
pieces.o  : pieces.c pieces.h gen.h readelf.h shdrtab.h \
            outbase.h outitems.h out.h
outbase.o : outbase.c outbase.h gen.h
outitems.o: outitems.c outitems.h gen.h outbase.h names.h pieces.h address.h
outelf32.o: outelf32.c outelf32.h gen.h outbase.h names.h outitems.h \
            address.h pieces.h shdrtab.h dynamic.h
outelf64.o: outelf64.c outelf64.h gen.h outbase.h names.h outitems.h \
            address.h pieces.h shdrtab.h dynamic.h
out.o     : out.c out.h gen.h readelf.h pieces.h address.h shdrtab.h \
            outbase.h outitems.h outelf32.h outelf64.h
elftoc.o  : elftoc.c gen.h readelf.h pieces.h address.h outbase.h out.h

elfnames.c: mknames
	echo '#include <elf.h>' | gcc -E -dM -xc /dev/stdin | ./$^ $@

mknames: mknames.o
mknames.o: mknames.c

clean:
	rm -f $(OBJS) elftoc elfnames.c mknames mknames.o
