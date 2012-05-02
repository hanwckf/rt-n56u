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

TARGETS=test_pwd test_pwd_glibc
TARGETS+=test_grp test_grp_glibc
TARGETS+=pwcat pwcat_glibc
TARGETS+=grcat grcat_glibc
TARGETS+=getgroups getgroups_glibc
TARGETS+=test_pwd_diff test_grp_diff pwcat_diff grcat_diff getgroups_diff

all: $(TARGETS)

test_pwd: test_pwd.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >test_pwd.out
	-@ echo " "

test_pwd_glibc: test_pwd.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >test_pwd_glibc.out
	-@ echo " "

test_grp: test_grp.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >test_grp.out
	-@ echo " "

test_grp_glibc: test_grp.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >test_grp_glibc.out
	-@ echo " "

pwcat: pwcat.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >pwcat.out
	-@ echo " "

pwcat_glibc: pwcat.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >pwcat_glibc.out
	-@ echo " "

grcat: grcat.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >grcat.out
	-@ echo " "

grcat_glibc: grcat.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >grcat_glibc.out
	-@ echo " "

getgroups: getgroups.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >getgroups.out
	-@ echo " "

getgroups_glibc: getgroups.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ 2>&1 >getgroups_glibc.out
	-@ echo " "

test_pwd_diff: test_pwd_glibc test_pwd
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u test_pwd_glibc.out test_pwd.out
	-@ echo " "

test_grp_diff: test_grp_glibc test_grp
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u test_grp_glibc.out test_grp.out
	-@ echo " "

pwcat_diff: pwcat_glibc pwcat
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u pwcat_glibc.out pwcat.out
	-@ echo " "

grcat_diff: grcat_glibc grcat
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u grcat_glibc.out grcat.out
	-@ echo " "

getgroups_diff: getgroups_glibc getgroups
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u getgroups_glibc.out getgroups.out
	-@ echo " "

clean:
	$(RM) *.[oa] *~ core $(TARGETS) *.out
