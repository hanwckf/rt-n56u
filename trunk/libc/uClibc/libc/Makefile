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

#--------------------------------------------------------
#
#There are a number of configurable options in "Config"
#
#--------------------------------------------------------

TOPDIR=../
include $(TOPDIR)Rules.mak

DIRS = misc pwd_grp stdio string termios inet signal stdlib sysdeps unistd

# Check if the target architecture has a version script for
# libc, and if so, include it when linking.
VERSION_SCRIPT:=${shell if [ -f sysdeps/linux/$(TARGET_ARCH)/libc.map ] ; then \
        echo "--version-script sysdeps/linux/$(TARGET_ARCH)/libc.map"; fi}

LIBNAME_TARGET:=$(TOPDIR)lib/$(LIBNAME)

all: halfclean $(LIBNAME_TARGET) $(DO_SHARED)

# Some functions are duplicated across subdirs, and when you pass $(AR)
# the same object file more than once, it'll add it to the archive multiple 
# times (boo!).  So what we do here is:
#  - import all the objects (thus we may have dupes)
#  - delete all the dupes
#  - re-import certain objects based upon preference
#    - the sysdeps dir should override all other symbols for example
# We need to use shell globbing with obj.* because if we use make's wildcard,
# the wildcard will be evaluated when `make` is run instead of when the make 
# target is evaluated.  That means if you run `rm obj.* ; make`, the wildcard 
# will evaluate to no files :(.
$(LIBNAME) shared_$(LIBNAME) ar-target: subdirs
	$(RM) $(LIBNAME) shared_$(LIBNAME)
	objs=`cat obj.*` ; \
	$(AR) $(ARFLAGS) $(LIBNAME) $$objs && \
	$(AR) dN 2 $(LIBNAME) $$objs && \
	$(AR) dN 2 $(LIBNAME) $$objs
	@for objfile in obj.signal \
	                obj.string obj.string.generic obj.string.$(TARGET_ARCH) \
	                obj.sysdeps.common obj.sysdeps.$(TARGET_ARCH) ; do \
		if [ -e $$objfile ] ; then \
			if [ "$(MAKE_IS_SILENT)" = "n" ] ; then \
				echo $(AR) $(ARFLAGS) $(LIBNAME) $$objfile ; \
			fi ; \
			objs=`cat $$objfile` ; \
		fi ; \
		$(AR) $(ARFLAGS) $(LIBNAME) $$objs || exit 1 ; \
	done
	cp $(LIBNAME) shared_$(LIBNAME)
	$(RANLIB) $(LIBNAME)

$(LIBNAME_TARGET): $(LIBNAME)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBNAME)
	$(INSTALL) -m 644 $(LIBNAME) $(TOPDIR)lib

shared: shared_$(LIBNAME)
	$(LD) $(LDFLAGS) $(VERSION_SCRIPT) -soname=$(SHARED_MAJORNAME) -o $(SHARED_FULLNAME) \
		--whole-archive shared_$(LIBNAME) \
		$(TOPDIR)libc/misc/internals/interp.o --no-whole-archive \
		-init __uClibc_init $(TOPDIR)lib/$(UCLIBC_LDSO) $(LIBGCC) $(LDADD_LIBFLOAT)
	@true #$(RM) -r tmp
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(SHARED_FULLNAME)
	$(INSTALL) -m 644 $(SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(SHARED_FULLNAME) $(TOPDIR)lib/libc.so
	$(LN) -sf $(SHARED_FULLNAME) $(TOPDIR)lib/$(SHARED_MAJORNAME)

halfclean:
	@$(RM) $(LIBNAME) shared_$(LIBNAME) uClibc_config.h
	@$(RM) $(SHARED_FULLNAME) $(SHARED_MAJORNAME) uClibc-0.* libc.so*

tags:
	ctags -R

clean: subdirs_clean halfclean
	@$(RM) -r tmp
	$(RM) include/asm include/linux include/bits
	$(RM) obj.*

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy subdirs
