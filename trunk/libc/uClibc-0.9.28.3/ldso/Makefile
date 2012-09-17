# Makefile for uClibc
#
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
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

DIRS = ldso libdl

all: ldso libdl

ifeq ($(strip $(BUILD_UCLIBC_LDSO)),y)
ldso: headers
	$(MAKE) -C ldso
libdl: ldso headers
	$(MAKE) -C libdl
shared: libdl
	$(MAKE) -C libdl shared
else
ldso libdl shared:
	echo "Skipping $@ target"
endif

LN_HEADERS      := $(patsubst %, include/%, elf.h)
LN_ARCH_HEADERS := $(patsubst %, include/%, dl-startup.h dl-syscalls.h dl-sysdep.h dl-debug.h)
HEADERS         := $(LN_HEADERS) $(LN_ARCH_HEADERS)
headers: $(HEADERS)
$(LN_HEADERS):
	$(LN) -fs $(TOPDIR)../$@ $@
$(LN_ARCH_HEADERS):
	$(LN) -fs ../ldso/$(TARGET_ARCH)/$(patsubst include/%,%,$@) $@

clean:
	set -e ; for d in $(DIRS) ; do $(MAKE) -C $$d $@ ; done
	-find . -name '*~' | xargs $(RM)
	$(RM) $(HEADERS)
