# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
# Copyright (C) 2004 Manuel Novoa III <mjn3@codepoet.org>
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
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../
include $(TOPDIR)Rules.mak

# Note: The *64.o objects are empty when compiled without large file support.

# SUSv3 functions
CSRC = fclose.c fcloseall.c fdopen.c fgetpos.c fopen.c freopen.c \
	fseeko.c fsetpos.c ftello.c getdelim.c getline.c gets.c getw.c \
	perror.c puts.c putw.c remove.c rewind.c setbuf.c setbuffer.c \
	setlinebuf.c setvbuf.c ungetc.c \
	printf.c vprintf.c vsprintf.c fprintf.c snprintf.c dprintf.c \
	asprintf.c sprintf.c vasprintf.c vdprintf.c vsnprintf.c \
	tmpfile.c tmpnam.c tmpnam_r.c popen.c tempnam.c ctermid.c

# getc -> alias for fgetc
# putc -> alias for fputc
# rename is a syscall

# Implementation support functions
CSRC += _READ.c _WRITE.c _adjust_pos.c _fopen.c _fwrite.c \
	_rfill.c _stdio.c _trans2r.c _trans2w.c _wcommit.c \
	_cs_funcs.c _load_inttype.c _store_inttype.c _uintmaxtostr.c
ifeq ($(strip $(UCLIBC_HAS_FLOATS)),y)
CSRC += _fpmaxtostr.c
endif

# stdio_ext.h functions
CSRC += __fbufsize.c __flbf.c __fpending.c __fpurge.c __freadable.c \
	__freading.c __fsetlocking.c __fwritable.c __fwriting.c _flushlbf.c

# Other glibc extensions
ifeq ($(strip $(UCLIBC_HAS_GLIBC_CUSTOM_STREAMS)),y)
CSRC += fopencookie.c fmemopen.c open_memstream.c
endif

# pthread functions
ifeq ($(strip $(UCLIBC_HAS_THREADS)),y)
CSRC += flockfile.c ftrylockfile.c funlockfile.c
endif

# Functions with unlocked versions
CUSRC = clearerr.c feof.c ferror.c fflush.c fgetc.c fgets.c fileno.c \
	fputc.c fputs.c fread.c fwrite.c getchar.c putchar.c
# getc_unlocked -> alias for fgetc_unlocked
# putc_unlocked -> alias for fputc_unlocked

# Largefile functions
CLOBJS = fgetpos64.o fopen64.o freopen64.o fseeko64.o fsetpos64.o ftello64.o
# tmpfile64.o

# vfprintf and support functions
MSRC2=	vfprintf.c
ifneq ($(USE_OLD_VFPRINTF),y)
MOBJ2=	vfprintf.o \
	_ppfs_init.o _ppfs_prepargs.o _ppfs_setargs.o _ppfs_parsespec.o \
	register_printf_function.o parse_printf_format.o
else
MOBJ2=
CSRC += old_vfprintf.c
endif

# vfscanf and support functions plus other *scanf funcs
MSRC3=	scanf.c
MOBJ3=	vfscanf.o __scan_cookie.o __psfs_parse_spec.o __psfs_do_numeric.o \
	scanf.o sscanf.o fscanf.o vscanf.o vsscanf.o

MWSRC=  wstdio.c
MWOBJ=

CWSRC =
ifeq ($(UCLIBC_HAS_WCHAR),y)
CWSRC += _wfwrite.c fwprintf.c swprintf.c vswprintf.c vwprintf.c wprintf.c \
	fwide.c ungetwc.c
CUSRC += fgetwc.c getwchar.c fgetws.c fputwc.c putwchar.c fputws.c
# getwc (fgetwc alias) getwc_unlocked (fgetwc_unlocked alias)
# putwc (fputwc alias) putwc_unlocked (fputwc_unlocked alias)
MOBJ2 += vfwprintf.o
MOBJ3 += wscanf.o swscanf.o fwscanf.o vwscanf.o vswscanf.o vfwscanf.o
endif

CSRC += $(CUSRC)

COBJS  = $(patsubst %.c,%.o, $(CSRC))
CUOBJS = $(patsubst %.c,%_unlocked.o, $(CUSRC))
CWOBJS = $(patsubst %.c,%.o, $(CWSRC))

ifeq ($(strip $(UCLIBC_HAS_WCHAR)),y)
COBJS += $(CWOBJS)
endif

OBJS = $(COBJS) $(CUOBJS) $(MOBJ2) $(MOBJ3) $(MWOBJ)

ifeq ($(strip $(UCLIBC_HAS_LFS)),y)
OBJS += $(CLOBJS)
endif

OBJ_LIST=../obj.stdio

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, stdio/%, $(OBJS)) > $(OBJ_LIST)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@

%_unlocked.o : %.c
	$(CC) $(CFLAGS) -D__DO_UNLOCKED -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@

%64.o : %.c
	$(CC) $(CFLAGS) -D__DO_LARGEFILE -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@

$(MOBJ2): $(MSRC2)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ3): $(MSRC3)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MWOBJ): $(MWSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

clean:
	$(RM) *.[oa] *~ core
