# Makefile for uClibc
#
# Copyright (C) 2005 Erik Andersen <andersen@uclibc.org>

include ../Rules.mak

# getrpcent_r_diff
TARGETS = getrpcent getrpcent_glibc getrpcent_diff

all: $(TARGETS)

getrpcent: getrpcent.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ > getrpcent.out
	-@ echo " "

getrpcent_glibc: getrpcent.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ > getrpcent_glibc.out
	-@ echo " "

getrpcent_diff: getrpcent getrpcent_glibc
	diff -u getrpcent_glibc.out getrpcent.out

clean:
	$(RM) *.[oa] *~ core $(TARGETS) *.out
