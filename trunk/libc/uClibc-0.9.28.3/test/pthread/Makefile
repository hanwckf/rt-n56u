# Makefile for uClibc
#
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include ../Rules.mak

LDFLAGS += 
#EXTRA_LIBS += -lc -lgcc -lpthread
EXTRA_LIBS += -lpthread

TARGETS=ex1 ex2 ex3 ex4 ex5 ex6 ex7
all: $(TARGETS)

ex1: ex1.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex2: ex2.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex3: ex3.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex4: ex4.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex5: ex5.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex6: ex6.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

ex7: ex7.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

clean:
	$(RM) *.[oa] *~ core $(TARGETS)
