# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
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
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../
include $(TOPDIR)Rules.mak


DIRS = assert ctype dirent file fnmatch internals \
       mntent syslog time utmp sysvipc statfs \
       error ttyent gnu search locale
ifeq ($(strip $(UCLIBC_HAS_REGEX)),y)
DIRS += regex
endif
ifeq ($(strip $(UCLIBC_HAS_WORDEXP)),y)
DIRS += wordexp
endif
ifeq ($(strip $(UCLIBC_HAS_THREADS)),y)
DIRS += pthread
endif
ifeq ($(strip $(UCLIBC_HAS_WCHAR)),y)
DIRS += wctype wchar
endif
ifeq ($(strip $(UCLIBC_HAS_GETTEXT_AWARENESS)),y)
DIRS += intl
endif
ifeq ($(strip $(UCLIBC_HAS_FTW)),y)
DIRS += ftw
endif
ifeq ($(strip $(UCLIBC_HAS_GLOB)),y)
DIRS += glob
endif

all: subdirs

tags:
	ctags -R

clean: subdirs_clean
	$(RM) *.[oa] *~ core

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
