# Rules.mak for uClibc test subdirs
#
# Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

.SUFFIXES:

top_builddir ?= ../

TESTDIR=$(top_builddir)test/

include $(top_builddir)/Rules.mak
ifndef TEST_INSTALLED_UCLIBC
ifdef UCLIBC_LDSO
ifeq (,$(findstring /,$(UCLIBC_LDSO)))
UCLIBC_LDSO := $(top_builddir)lib/$(UCLIBC_LDSO)
endif
else
UCLIBC_LDSO := $(firstword $(wildcard $(top_builddir)lib/ld*))
endif
endif
#--------------------------------------------------------
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL:= C
export LC_ALL

ifeq ($(strip $(TARGET_ARCH)),)
TARGET_ARCH:=$(shell $(CC) -dumpmachine | sed -e s'/-.*//' \
	-e 's/i.86/i386/' \
	-e 's/sun.*/sparc/' -e 's/sparc.*/sparc/' \
	-e 's/sa110/arm/' -e 's/arm.*/arm/g' \
	-e 's/m68k.*/m68k/' \
	-e 's/parisc.*/hppa/' \
	-e 's/ppc/powerpc/g' \
	-e 's/v850.*/v850/g' \
	-e 's/sh[234]/sh/' \
	-e 's/mips.*/mips/' \
	-e 's/cris.*/cris/' \
	-e 's/xtensa.*/xtensa/' \
	)
endif
export TARGET_ARCH

RM_R = $(Q)$(RM) -r
LN_S = $(Q)$(LN) -fs

ifneq ($(KERNEL_HEADERS),)
ifeq ($(patsubst /%,/,$(KERNEL_HEADERS)),/)
# Absolute path in KERNEL_HEADERS
KERNEL_INCLUDES += -I$(KERNEL_HEADERS)
else
# Relative path in KERNEL_HEADERS
KERNEL_INCLUDES += -I$(top_builddir)$(KERNEL_HEADERS)
endif
endif

XCOMMON_CFLAGS := -I$(top_builddir)test -D_GNU_SOURCE
XWARNINGS      += $(CFLAG_-Wstrict-prototypes)
CFLAGS         := -nostdinc -I$(top_builddir)$(LOCAL_INSTALL_PATH)/usr/include
CFLAGS         += $(XCOMMON_CFLAGS) $(KERNEL_INCLUDES) $(CC_INC)
CFLAGS         += $(OPTIMIZATION) $(CPU_CFLAGS) $(XWARNINGS)

# Can't add $(OPTIMIZATION) here, it may be target-specific.
# Just adding -Os for now.
HOST_CFLAGS    += $(XCOMMON_CFLAGS) -Os $(XWARNINGS) -std=gnu99

LDFLAGS        := $(CPU_LDFLAGS-y) -Wl,-z,now
ifeq ($(DODEBUG),y)
	CFLAGS        += -g
	HOST_CFLAGS   += -g
	LDFLAGS       += -Wl,-g
	HOST_LDFLAGS  += -Wl,-g
else
	LDFLAGS       += -Wl,-s
	HOST_LDFLAGS  += -Wl,-s
endif

ifneq ($(HAVE_SHARED),y)
	LDFLAGS       += -Wl,-static -static-libgcc
endif

LDFLAGS += -B$(top_builddir)lib -Wl,-rpath,$(top_builddir)lib -Wl,-rpath-link,$(top_builddir)lib
UCLIBC_LDSO_ABSPATH=$(shell pwd)
ifdef TEST_INSTALLED_UCLIBC
LDFLAGS += -Wl,-rpath,./
UCLIBC_LDSO_ABSPATH=$(RUNTIME_PREFIX)$(MULTILIB_DIR)
endif

ifeq ($(findstring -static,$(LDFLAGS)),)
LDFLAGS += -Wl,--dynamic-linker,$(UCLIBC_LDSO_ABSPATH)/$(UCLIBC_LDSO)
endif

ifeq ($(LDSO_GNU_HASH_SUPPORT),y)
# Check for binutils support is done on root Rules.mak
LDFLAGS += $(CFLAG_-Wl--hash-style=gnu)
endif


ifneq ($(findstring -s,$(MAKEFLAGS)),)
DISP := sil
Q    := @
SCAT := -@true
else
ifneq ($(V)$(VERBOSE),)
DISP := ver
Q    :=
SCAT := cat
else
DISP := pur
Q    := @
SCAT := -@true
endif
endif
ifneq ($(Q),)
MAKEFLAGS += --no-print-directory
endif

banner := ---------------------------------
pur_showclean = echo "  "CLEAN $(notdir $(CURDIR))
pur_showdiff  = echo "  "TEST_DIFF $(notdir $(CURDIR))/
pur_showlink  = echo "  "TEST_LINK $(notdir $(CURDIR))/ $@
pur_showtest  = echo "  "TEST_EXEC $(notdir $(CURDIR))/ $(@:.exe=)
sil_showclean =
sil_showdiff  = true
sil_showlink  = true
sil_showtest  = true
ver_showclean =
ver_showdiff  = true echo
ver_showlink  = true echo
ver_showtest  = printf "\n$(banner)\nTEST $(notdir $(CURDIR))/ $(@:.exe=)\n$(banner)\n"
do_showclean  = $($(DISP)_showclean)
do_showdiff   = $($(DISP)_showdiff)
do_showlink   = $($(DISP)_showlink)
do_showtest   = $($(DISP)_showtest)
showclean = @$(do_showclean)
showdiff  = @$(do_showdiff)
showlink  = @$(do_showlink)
showtest  = @$(do_showtest)
