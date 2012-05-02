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

CFLAGS += -D_GNU_SOURCE
GLIBC_CFLAGS += -D_GNU_SOURCE

TARGETS=signal signal_glibc sigchld sigchld_glibc 
all: $(TARGETS)

signal: signal.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

signal_glibc: signal.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

sigchld: sigchld.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

sigchld_glibc: sigchld.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@
	-@ echo " "

clean:
	$(RM) *.[oa] *~ core $(TARGETS)
