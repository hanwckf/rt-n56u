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

TOPDIR=../../../
include $(TOPDIR)Rules.mak

ALL_SUBDIRS = arm common cris h8300 i386 m68k mips nios nios2 powerpc sh sh64 sparc v850

all: common $(TARGET_ARCH)

tags:
	ctags -R

common: dummy
	$(MAKE) -C common

$(TARGET_ARCH): common
	$(MAKE) -C $(TARGET_ARCH)

clean: subdirs_clean

subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy $(TARGET_ARCH)
