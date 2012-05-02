# Makefile for uClibc's pthread library
#
# Copyright (C) 2003 Paul Mundt <lethal@linux-sh.org>
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
# Makefile for uClibc

TOPDIR=../../../
include $(TOPDIR)Rules.mak

# Generally most TARGET_ARCHs aren't going to want a make rule here as they
# can generally just inline directly into the upper-level pt-machine.c. As
# such, we offer this capability for platforms that either can't get away
# with the inlining, or need to be able to do builds here for other reasons. 

DIRS =

ifeq ($(TARGET_ARCH),sh64)
	DIRS += $(TARGET_ARCH)
endif

all: subdirs

clean: subdirs_clean
	$(RM) *~ core

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy subdirs

