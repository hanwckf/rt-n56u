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

TOPDIR=../
include $(TOPDIR)Rules.mak

CFLAGS+=$(SSP_ALL_CFLAGS) -B../lib
# This isnt the correct variable, but hijacking it for now is fine by me as it works
LDPIEFLAG+=../lib/$(UCLIBC_LDSO)

TARGETS = ldd ldconfig

ifeq ($(strip $(UCLIBC_HAS_LOCALE)),y)
TARGET_ICONV = iconv
else
TARGET_ICONV =
endif

ifeq ($(strip $(LDSO_CACHE_SUPPORT)),y)
HOST_LDSO_CACHE_FLAG = -D__LDSO_CACHE_SUPPORT__=1
else
HOST_LDSO_CACHE_FLAG =
endif

# NOTE: We build the utils AFTER we have a uClibc-targeted toolchain.

ifeq ($(strip $(HAVE_SHARED)),y)
all: $(TARGETS) $(TARGET_ICONV)
else
all: $(TARGET_ICONV)
endif

headers:
	@$(LN) -fs $(TOPDIR)include/elf.h

readelf.c ldconfig.c ldd.c:	headers

readelf: readelf.c
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIPTOOL) -s -x -R .note -R .comment $@

ifeq ($(strip $(UCLIBC_STATIC_LDCONFIG)),y)
LDCONFIG_CFLAGS := -static
else
LDCONFIG_CFLAGS := $(PIEFLAG) $(LDPIEFLAG)
endif
ldconfig: ldconfig.c chroot_realpath.c
	$(CC) $(CFLAGS) $(LDCONFIG_CFLAGS) \
		-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) -I. -I../ldso/include \
		$^ -o $@
	$(STRIPTOOL) -s -x -R .note -R .comment $@

ldd: ldd.c
	$(CC) $(CFLAGS) $(PIEFLAG) $(LDPIEFLAG) \
		-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) -I. -I../ldso/include \
		$^ -o $@
	$(STRIPTOOL) -s -x -R .note -R .comment $@

iconv: ../libc/misc/wchar/wchar.c
	$(CC) $(CFLAGS) $(PIEFLAG) $(LDPIEFLAG) \
		-DL_iconv_main \
		$^ -o $@
	$(STRIPTOOL) -s -x -R .note -R .comment $@

ifeq ($(strip $(HAVE_SHARED)),y)
hostutils: ldd.host ldconfig.host readelf.host
else
hostutils: readelf.host
endif

ldd.host: ldd.c
	$(HOSTCC) $(HOSTCFLAGS) -Wl,-s \
 		-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" $(HOST_LDSO_CACHE_FLAG) \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) -I. -I../ldso/include \
		$^ -o $@

ldconfig.host: ldconfig.c chroot_realpath.c
	$(HOSTCC) $(HOSTCFLAGS) -Wl,-s \
 		-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" $(HOST_LDSO_CACHE_FLAG) \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) -I. -I../ldso/include \
		$^ -o $@

readelf.host: readelf.c
	$(HOSTCC) $(HOSTCFLAGS) -Wl,-s $^ -o $@

clean:
	$(RM) $(TARGETS) *.o *~ core *.target elf.h iconv *.host

install: all
ifeq ($(strip $(HAVE_SHARED)),y)
	$(INSTALL) -d $(PREFIX)$(RUNTIME_PREFIX)sbin
	$(INSTALL) -d $(PREFIX)$(RUNTIME_PREFIX)usr/bin
	$(INSTALL) -m 755 ldd $(PREFIX)$(RUNTIME_PREFIX)usr/bin/ldd
	$(INSTALL) -m 755 ldconfig $(PREFIX)$(RUNTIME_PREFIX)sbin/ldconfig;
	#$(INSTALL) -m 755 readelf $(PREFIX)$(RUNTIME_PREFIX)usr/bin/readelf
endif
ifeq ($(strip $(UCLIBC_HAS_LOCALE)),y)
	$(INSTALL) -m 755 iconv $(PREFIX)$(RUNTIME_PREFIX)usr/bin/iconv
endif
