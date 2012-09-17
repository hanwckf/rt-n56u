# Rules.mak for uClibc test subdirs
#
# Copyright (C) 2001 by Lineo, inc.
#
# Note: This does not read the top level Rules.mak file
#

TOPDIR = ../../
TESTDIR=$(TOPDIR)test/

-include $(TOPDIR).config

ifndef UCLIBC_LDSO
UCLIBC_LDSO := ld-uClibc.so.0
endif

#--------------------------------------------------------
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc. 
LC_ALL:= C
export LC_ALL

ifeq ($(strip $(TARGET_ARCH)),)
TARGET_ARCH:=$(shell $(CC) -dumpmachine | sed -e s'/-.*//' \
	-e 's/i.86/i386/' \
	-e 's/sparc.*/sparc/' \
	-e 's/arm.*/arm/g' \
	-e 's/m68k.*/m68k/' \
	-e 's/ppc/powerpc/g' \
	-e 's/v850.*/v850/g' \
	-e 's/sh[234]/sh/' \
	-e 's/mips-.*/mips/' \
	-e 's/mipsel-.*/mipsel/' \
	-e 's/cris.*/cris/' \
	)
endif
export TARGET_ARCH


#--------------------------------------------------------
# If you are running a cross compiler, you will want to set 'CROSS'
# to something more interesting...  Target architecture is determined
# by asking the CC compiler what arch it compiles things for, so unless
# your compiler is broken, you should not need to specify TARGET_ARCH
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS=mipsel-linux-
# will build uClibc for 'mipsel'.

CROSS      = $(subst ",, $(strip $(CROSS_COMPILER_PREFIX)))
CC         = $(CROSS)gcc
STRIPTOOL  = strip
RM         = rm -f
ifeq ($(LDSO_LDD_SUPPORT),y)
LDD        = $(TOPDIR)utils/ldd
else
LDD        = @true
endif

# Select the compiler needed to build binaries for your development system
HOSTCC     = gcc
HOSTCFLAGS = -O2 -Wall


#--------------------------------------------------------
# Check if 'ls -sh' works or not
LSFLAGS = -l

# A nifty macro to make testing gcc features easier
check_gcc=$(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)

# use '-Os' optimization if available, else use -O2, allow Config to override
# Override optimization settings when debugging
ifeq ($(DODEBUG),y)
OPTIMIZATION    = -O0
else
OPTIMIZATION   += $(call check_gcc,-Os,-O2)
endif

XWARNINGS       = $(subst ",, $(strip $(WARNINGS))) -Wstrict-prototypes
XARCH_CFLAGS    = $(subst ",, $(strip $(ARCH_CFLAGS)))
CFLAGS          = $(XWARNINGS) $(OPTIMIZATION) $(XARCH_CFLAGS)
GLIBC_CFLAGS   += $(XWARNINGS) $(OPTIMIZATION)
LDFLAGS         = 

ifeq ($(DODEBUG),y)
	CFLAGS        += -g
	GLIBC_CFLAGS  += -g
	LDFLAGS       += -g -Wl,-warn-common
	GLIBC_LDFLAGS  = -g -Wl,-warn-common 
	STRIPTOOL      = true -Since_we_are_debugging
else
	LDFLAGS       += -s -Wl,-warn-common
	GLIBC_LDFLAGS  = -s -Wl,-warn-common
	STRIP          = $(STRIPTOOL) --remove-section=.note --remove-section=.comment $(PROG)
endif

ifneq ($(strip $(HAVE_SHARED)),y)
	LDFLAGS       += -static
	GLIBC_LDFLAGS += -static
else
	LDFLAGS       += -Wl,-dynamic-linker,$(TOPDIR)lib/$(UCLIBC_LDSO)
endif
