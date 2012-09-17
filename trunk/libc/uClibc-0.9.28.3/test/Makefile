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

TOPDIR=../
-include $(TOPDIR).config

.EXPORT_ALL_VARIABLES:


ALL_SUBDIRS = \
	args assert crypt ctype pwd_grp signal silly stdlib string unistd \
	#misc
DIRS := $(ALL_SUBDIRS)

ifeq ($(HAVE_SHARED)$(BUILD_UCLIBC_LDSO),yy)
	DIRS += dlopen
endif
ifeq ($(UCLIBC_HAS_THREADS),y)
	DIRS += pthread
endif
ALL_SUBDIRS += pthread dlopen


all: subdirs

tags:
	ctags -R

clean: subdirs_clean
	$(RM) *.[oa] *~ core

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	@echo "Making test in $(patsubst _dir_%,%,$@)"
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
