# Makefile for uClibc
#
# Copyright (C) 2000-2003 Erik Andersen <andersen@uclibc.org>
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

DEBUG_LIBS=X
#DEBUG_LIBS=LD_DEBUG

all: run

test1: test1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test1 test1.c -ldl

test2: test2.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test2 test2.c -ldl

test3: test3.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test3 test3.c -ldl ./libtest1.so ./libtest2.so

libtest1.o: libtest1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -c libtest1.c -o libtest1.o

libtest2.o: libtest2.c
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -c libtest2.c -o libtest2.o

libtest1.so: libtest1.o
	$(CC) $(CFLAGS) -fPIC -shared -o libtest1.so -Wl,-soname,libtest1.so libtest1.o ./libtest2.so

libtest2.so: libtest2.o
	$(CC) $(CFLAGS) -fPIC -shared -o libtest2.so -Wl,-soname,libtest2.so libtest2.o

dltest: dltest.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DLIBNAME="\"./libtest.so\"" dltest.c -ldl -lpthread -o dltest

libtest.so: libtest.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,libtest.so libtest.c -o libtest.so

# Second time, directly link libtest3.so with libpthread
dltest2: dltest.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DLIBNAME="\"./libtest3.so\"" dltest.c -ldl -lpthread -o dltest2

libtest3.so: libtest.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,libtest3.so libtest.c -o libtest3.so -lpthread

run: libtest2.so libtest1.so test1 test2 test3 dltest libtest.so dltest2 libtest3.so
	@echo "----------running test 1--------------"
	LD_LIBRARY_PATH=`pwd`:. $(DEBUG_LIBS)=all ./test1
	@echo "----------running test 2--------------"
	LD_LIBRARY_PATH=`pwd`:. $(DEBUG_LIBS)=all ./test2
	@echo "----------running test 3--------------"
	LD_LIBRARY_PATH=`pwd`:. $(DEBUG_LIBS)=all ./test3
	@echo "----------running test 3--------------"
	$(DEBUG_LIBS)=all ./dltest2
	@echo "----------running test 4--------------"
	$(DEBUG_LIBS)=all ./dltest

clean:
	$(RM) *.o libtest1.so* libtest2.so* test1 test2 test3 \
		dltest dltest2 libtest.so libtest3.so
