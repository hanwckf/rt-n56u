# Rules.mak for uClibc-ng
#
# Copyright (C) 2000-2008 Erik Andersen <andersen@uclibc.org>
# Copyright (C) 2015-2018 Waldemar Brodkorb <wbx@uclibc-ng.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# make nano-doc
# FOO = bar  -- recursively expanded variable. Value is remebered verbatim.
#               If it contains references to other variables, these references
#               are expanded whenever this variable is _substituted_.
# FOO := bar -- simply expanded variable. Right hand is expanded when
#               the variable is _defined_. Therefore faster than =.
# FOO ?= bar -- set a value only if it is not already set
#               (behaves as =, not :=).
# FOO += bar -- append; if FOO is not defined, acts like = (not :=).


# check for proper make version
ifneq ($(findstring x3.7,x$(MAKE_VERSION)),)
$(error Your make is too old $(MAKE_VERSION). Go get at least 3.80)
endif

#-----------------------------------------------------------
# This file contains rules which are shared between multiple
# Makefiles.  All normal configuration options live in the
# file named ".config".  Don't mess with this file unless
# you know what you are doing.

clean_targets := clean realclean distclean \
	objclean-y headers_clean-y CLEAN_utils
noconfig_targets := menuconfig config nconfig \
	oldaskconfig silentoldconfig oldconfig allnoconfig allyesconfig \
	alldefconfig randconfig defconfig savedefconfig listnewconfig \
	olddefconfig \
	xconfig gconfig update-po-config mconf qconf gconf nconf conf \
	release dist tags help


#-----------------------------------------------------------
# If you are running a cross compiler, you will want to set
# 'CROSS_COMPILE' to something more interesting ...  Target
# architecture is determined by asking the CC compiler what
# arch it compiles things for, so unless your compiler is
# broken, you should not need to specify TARGET_ARCH.
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS_COMPILE=arm-linux-
# will build uClibc-ng for 'arm'.
# CROSS is still supported for backward compatibily only

CROSS_COMPILE ?= $(CROSS)

CC         = $(CROSS_COMPILE)gcc
AR         = $(CROSS_COMPILE)ar
LD         = $(CROSS_COMPILE)ld
NM         = $(CROSS_COMPILE)nm
OBJDUMP    = $(CROSS_COMPILE)objdump
STRIPTOOL  = $(CROSS_COMPILE)strip

INSTALL    = install
LN         = ln
RM         = rm -f
TAR        = tar
SED        = sed
AWK        = awk

STRIP_FLAGS ?= -x -R .note -R .comment

# Select the compiler needed to build binaries for your development system
HOSTCC     = gcc
BUILD_CFLAGS = -Os

#---------------------------------------------------------
# Nothing beyond this point should ever be touched by mere
# mortals.  Unless you hang out with the gods, you should
# probably leave all this stuff alone.

# strip quotes
qstrip = $(strip $(subst ",,$(1)))
#"))

# kconfig stuff
KCONFIG_CONFIG ?= $(top_builddir).config
KCONFIG_CONFIG := $(abspath $(KCONFIG_CONFIG))
export KCONFIG_CONFIG
KCONFIG_AUTOCONFIG := $(dir $(KCONFIG_CONFIG))include/config/auto.conf
export KCONFIG_AUTOCONFIG
KCONFIG_TRISTATE := $(dir $(KCONFIG_CONFIG))include/config/tristate.conf
export KCONFIG_TRISTATE
srctree := $(abspath $(top_srcdir))
export srctree
KCONFIG_AUTOHEADER := $(dir $(KCONFIG_CONFIG))include/generated/autoconf.h
export KCONFIG_AUTOHEADER
Kconfig := $(abspath $(top_srcdir)extra/Configs/Config.in)

# Pull in the user's uClibc-ng configuration
ifeq ($(filter $(noconfig_targets) clean CLEAN_%,$(MAKECMDGOALS)),)
-include $(KCONFIG_CONFIG)
endif

TARGET_ARCH:=$(call qstrip,$(TARGET_ARCH))
ifeq ($(TARGET_ARCH),)
ARCH ?= $(shell uname -m | $(SED) -e s/i.86/i386/ \
				  -e s/sun.*/sparc/ -e s/sparc.*/sparc/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/nds32.*/nds32/ \
				  -e s/sh.*/sh/ \
				  -e s/s390x/s390/ -e s/parisc.*/hppa/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/xtensa.*/xtensa/ )
else
ARCH = $(TARGET_ARCH)
endif
export ARCH

# Make certain these contain a final "/", but no "//"s.
scrub_path = $(strip $(subst //,/, $(subst ,/, $(call qstrip,$(1)))))
TARGET_SUBARCH := $(call qstrip,$(TARGET_SUBARCH))
RUNTIME_PREFIX := $(call scrub_path,$(RUNTIME_PREFIX))
DEVEL_PREFIX   := $(call scrub_path,$(DEVEL_PREFIX))
MULTILIB_DIR   := $(call scrub_path,$(MULTILIB_DIR))
KERNEL_HEADERS := $(call scrub_path,$(KERNEL_HEADERS))
export RUNTIME_PREFIX DEVEL_PREFIX KERNEL_HEADERS MULTILIB_DIR


# Now config hard core
MAJOR_VERSION := 1
MINOR_VERSION := 0
SUBLEVEL      := 32
EXTRAVERSION  :=
VERSION       := $(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)
ABI_VERSION   := $(MAJOR_VERSION)
ifneq ($(EXTRAVERSION),)
VERSION       := $(VERSION)$(EXTRAVERSION)
endif
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL := C
export MAJOR_VERSION MINOR_VERSION SUBLEVEL VERSION ABI_VERSION LC_ALL

LIBC := libc
SHARED_LIBNAME := $(LIBC).so.$(ABI_VERSION)

UCLIBC_LDSO_NAME := ld-uClibc
ARCH_NATIVE_BIT := 32
ifneq ($(findstring  $(TARGET_ARCH) , hppa64 ia64 powerpc64 s390x sparc64 x86_64 ),)
UCLIBC_LDSO_NAME := ld64-uClibc
ARCH_NATIVE_BIT := 64
else
ifeq ($(CONFIG_MIPS_N64_ABI),y)
UCLIBC_LDSO_NAME := ld64-uClibc
ARCH_NATIVE_BIT := 64
endif
endif

UCLIBC_LDSO := $(UCLIBC_LDSO_NAME).so.$(ABI_VERSION)
NONSHARED_LIBNAME := uclibc_nonshared.a
NONSHARED_LIBPTHREAD := libpthread_nonshared.a

libc := $(top_builddir)lib/$(SHARED_LIBNAME)
libc.depend := $(top_builddir)lib/$(SHARED_LIBNAME:.$(ABI_VERSION)=)
interp := $(top_builddir)lib/interp.os
ldso := $(top_builddir)lib/$(UCLIBC_LDSO)
headers_dep := $(top_builddir)include/bits/sysnum.h \
	$(top_builddir)include/bits/uClibc_config.h
sub_headers := $(headers_dep)

LIBS := $(interp) -L$(top_builddir)lib $(libc:.$(ABI_VERSION)=)

# Make sure DESTDIR and PREFIX can be used to install
# PREFIX is a uClibcism while DESTDIR is a common GNUism
ifndef PREFIX
PREFIX = $(DESTDIR)
endif

ifneq ($(HAVE_SHARED),y)
libc :=
interp :=
ldso :=
endif

comma:=,
space:= #

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=$(call qstrip,$(CROSS_COMPILER_PREFIX))
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)
check_as=$(shell \
	if $(CC) -Wa,$(1) -Wa,-Z -c -o /dev/null -xassembler /dev/null > /dev/null 2>&1; \
	then echo "-Wa,$(1)"; fi)
check_ld=$(shell \
	tf="/tmp/clducl$$$$.c"; echo "int _start(){return 0;}int main(){return 0;}" >$$tf; \
	if $(CC) $(LDFLAG-fuse-ld) $(CFLAG_-Wl--no-warn-mismatch) -Wl,$(1) $(CFLAG_-nostdlib) -o /dev/null $$tf > /dev/null 2>&1; \
	then echo "$(1)"; fi; rm -f $$tf )

# Use variable indirection here so that we can have variable
# names with fun chars in them like equal signs
define check-tool-var
ifeq ($(filter $(clean_targets) CLEAN_%,$(MAKECMDGOALS)),)
_v = $(2)_$(3)
ifndef $$(_v)
$$(_v) := $$(call $(1),$(subst %, ,$(3)))
export $$(_v)
endif
endif
endef

# Usage: check-gcc-var,<flag>
# Check the C compiler to see if it supports <flag>.
# Export the variable CFLAG_<flag> if it does.
define check-gcc-var
$(call check-tool-var,check_gcc,CFLAG,$(1))
endef
# Usage: check-as-var,<flag>
# Check the assembler to see if it supports <flag>.  Export the
# variable ASFLAG_<flag> if it does (for invoking the assembler),
# as well CFLAG_-Wa<flag> (for invoking the compiler driver).
define check-as-var
$(call check-tool-var,check_as,ASFLAG,$(1))
_v = CFLAG_-Wa$(1)
export $$(_v) = $$(if $$(ASFLAG_$(1)),-Wa$$(comma)$$(ASFLAG_$(1)))
endef
# Usage: check-ld-var,<flag>
# Check the linker to see if it supports <flag>.  Export the
# variable LDFLAG_<flag> if it does (for invoking the linker),
# as well CFLAG_-Wl<flag> (for invoking the compiler driver).
define check-ld-var
$(call check-tool-var,check_ld,LDFLAG,$(1))
_v = CFLAG_-Wl$(1)
export $$(_v) = $$(if $$(LDFLAG_$(1)),-Wl$$(comma)$$(LDFLAG_$(1)))
endef
# Usage: cache-output-var,<variable>,<shell command>
# Execute <shell command> and cache the output in <variable>.
define cache-output-var
ifndef $(1)
$(1) := $$(shell $(2))
export $(1)
endif
endef


ARFLAGS:=cr

# Note: The check for -nostdlib has to be before all calls to check_ld
$(eval $(call check-gcc-var,-nostdlib))
$(eval $(call check-gcc-var,-nostartfiles))
# deliberately not named CFLAG-fuse-ld since unchecked and from user
LDFLAG-fuse-ld := $(filter -fuse-ld=%,$(call qstrip,$(UCLIBC_EXTRA_CFLAGS)))
# failed to merge target specific data of file /dev/null
# Could use -Wl,--script,$(top_srcdir)extra/scripts/none.lds as well.
$(eval $(call check-ld-var,--no-warn-mismatch))

$(eval $(call cache-output-var,GCC_VER,$(CC) -dumpversion))
GCC_VER := $(subst ., ,$(GCC_VER))
GCC_MAJOR_VER ?= $(word 1,$(GCC_VER))
GCC_MINOR_VER ?= $(word 2,$(GCC_VER))

# Flags in OPTIMIZATION are used only for non-debug builds

OPTIMIZATION:=
OPTIMIZATION-$(GCC_MAJOR_VER):=
OPTIMIZATION-$(GCC_MAJOR_VER).$(GCC_MINOR_VER):=

# Use '-Os' optimization if available, else use -O2, allow Config to override
$(eval $(call check-gcc-var,-Os))
ifneq ($(CFLAG_-Os),)
OPTIMIZATION += $(CFLAG_-Os)
else
$(eval $(call check-gcc-var,-O2))
OPTIMIZATION += $(CFLAG_-O2)
endif
# Use the gcc 3.4 -funit-at-a-time optimization when available
$(eval $(call check-gcc-var,-funit-at-a-time))
OPTIMIZATION-3.4 += $(CFLAG_-funit-at-a-time)
$(eval $(call check-gcc-var,-fstrict-aliasing))
OPTIMIZATION += $(CFLAG_-fstrict-aliasing)

# CPU_CFLAGS-y contain options which are not warnings,
# not include or library paths, and not optimizations.

# Why -funsigned-char: I hunted a bug related to incorrect
# sign extension of 'char' type for 10 hours straight. Not fun.
CPU_CFLAGS-y := -funsigned-char -fno-builtin

$(eval $(call check-gcc-var,-fno-asm))
CPU_CFLAGS-y += $(CFLAG_-fno-asm)
$(eval $(call check-gcc-var,-fmerge-all-constants))
CPU_CFLAGS-y += $(CFLAG_-fmerge-all-constants)

ifeq ($(UCLIBC_HAS_SOFT_FLOAT),y)
ifneq ($(TARGET_ARCH),bfin)
ifneq ($(TARGET_ARCH),lm32)
ifneq ($(TARGET_ARCH),nios2)
ifneq ($(TARGET_ARCH),nds32)
ifneq ($(TARGET_ARCH),sh)
ifneq ($(TARGET_ARCH),c6x)
ifneq ($(TARGET_ARCH),h8300)
ifneq ($(TARGET_ARCH),arc)
ifneq ($(TARGET_ARCH),aarch64)
CPU_CFLAGS-y += -msoft-float
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(TARGET_ARCH),aarch64)
CPU_CFLAGS-y += -ftls-model=initial-exec
endif

$(eval $(call check-gcc-var,-std=gnu99))
CPU_CFLAGS-y += $(CFLAG_-std=gnu99)

CPU_CFLAGS-$(UCLIBC_FORMAT_SHARED_FLAT) += -mid-shared-library
CPU_CFLAGS-$(UCLIBC_FORMAT_FLAT_SEP_DATA) += -msep-data

CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN) += -Wl,-EL
CPU_LDFLAGS-$(ARCH_BIG_ENDIAN)    += -Wl,-EB

PICFLAG-y := -fPIC
PICFLAG-$(UCLIBC_FORMAT_FDPIC_ELF) := -mfdpic
PICFLAG-$(UCLIBC_FORMAT_DSBT_ELF)  := -mdsbt -fpic
PICFLAG := $(PICFLAG-y)
PIEFLAG_NAME:=-fPIE

$(eval $(call check-gcc-var,-fdata-sections))
$(eval $(call check-gcc-var,-ffunction-sections))

# Some nice CPU specific optimizations
ifeq ($(TARGET_ARCH),i386)
$(eval $(call check-gcc-var,-fomit-frame-pointer))
	OPTIMIZATION += $(CFLAG_-fomit-frame-pointer)

ifeq ($(CONFIG_386)$(CONFIG_486)$(CONFIG_586),y)
	# TODO: Change this to a gcc version check.  This bug
	# should be fixed with at least gcc-4.3.
	# Non-SSE capable processor.
	# NB: this may make SSE insns segfault!
	# -O1 -march=pentium3, -Os -msse etc are known to be affected.
	# See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=13685
	# -m32 is needed if host is 64-bit
	OPTIMIZATION+=$(call check_gcc,-m32 -mpreferred-stack-boundary=2,)
else
$(eval $(call check-gcc-var,-mpreferred-stack-boundary=4))
	OPTIMIZATION += $(CFLAG_-mpreferred-stack-boundary=4)
endif

	# Choice of alignment (please document why!)
	#  -falign-labels: in-line labels
	#  (reachable by normal code flow, aligning will insert nops
	#  which will be executed - may even make things slower)
	#  -falign-jumps: reachable only by a jump
	# Generic: no alignment at all (smallest code)
	GCC_FALIGN=$(call check_gcc,-falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1,-malign-jumps=1 -malign-loops=1)
	OPTIMIZATION+=$(GCC_FALIGN)

	# Putting each function and data object into its own section
	# allows for kbytes of less text if users link against static uclibc
	# using ld --gc-sections.
	# ld 2.18 can't do that (yet?) for shared libraries, so we itself
	# do not use --gc-sections at shared lib link time.
	# However, in combination with sections being sorted by alignment
	# it does result in much reduced padding:
	#   text    data     bss     dec     hex
	# 235319    1472    5992  242783   3b45f old.so
	# 234104    1472    5980  241556   3af94 new.so
	# Without -ffunction-sections, all functions will get aligned
	# to 4 byte boundary by as/ld. This is arguably a bug in as.
	# It specifies 4 byte align for .text even if not told to do so:
	# Idx Name          Size      VMA       LMA       File off  Algn
	#   0 .text         xxxxxxxx  00000000  00000000  xxxxxxxx  2**2 <===!
	CPU_CFLAGS-y  += $(CFLAG_-ffunction-sections) $(CFLAG_-fdata-sections)
	CPU_LDFLAGS-y += $(CFLAG_-Wl--sort-common)
$(eval $(call check-ld-var,--sort-section=alignment))
	CPU_LDFLAGS-y += $(CFLAG_-Wl--sort-section=alignment)

	CPU_LDFLAGS-y+=-m32
	CPU_CFLAGS-y+=-m32
endif

ifeq ($(TARGET_ARCH),sparc)
	CPU_CFLAGS-$(CONFIG_SPARC_V7)+=-mcpu=v7
	CPU_CFLAGS-$(CONFIG_SPARC_V8)+=-mcpu=v8
	CPU_CFLAGS-$(CONFIG_SPARC_V9)+=-mcpu=v9
	CPU_CFLAGS-$(CONFIG_SPARC_V9B)+=$(call check_gcc,-mcpu=v9b,-mcpu=ultrasparc)
endif

ifeq ($(TARGET_ARCH),arm)
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)+=-mlittle-endian
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)+=-mbig-endian
endif

ifeq ($(TARGET_ARCH),metag)
        SYMBOL_PREFIX=_
        CPU_CFLAGS-$(CONFIG_META_1_2)+=
        CPU_CFLAGS-$(CONFIG_META_2_1)+=-Wa,-mcpu=metac21
endif

ifeq ($(TARGET_ARCH),mips)
	OPTIMIZATION+=-mno-split-addresses
	CPU_CFLAGS-$(CONFIG_MIPS_N64_ABI)+=-mabi=64
	CPU_CFLAGS-$(CONFIG_MIPS_O32_ABI)+=-mabi=32
	CPU_CFLAGS-$(CONFIG_MIPS_N32_ABI)+=-mabi=n32
	CPU_CFLAGS-$(CONFIG_MIPS_NAN_LEGACY)+=$(call check_gcc,-mnan=legacy)
	CPU_CFLAGS-$(CONFIG_MIPS_NAN_2008)+=$(call check_gcc,-mnan=2008)
	CPU_LDFLAGS-y += $(CPU_CFLAGS)
endif

ifeq ($(TARGET_ARCH),nds32)
	CPU_CFLAGS-$(CONFIG_NDS32_ISA)+=-march=nds32
	CFLAGS-.os+= -DPIC
	CFLAGS-.oS+= -DPIC
endif

ifeq ($(TARGET_ARCH),sh)
$(eval $(call check-gcc-var,-mprefergot))
	OPTIMIZATION += $(CFLAG_-mprefergot)
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)+=-ml
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)+=-mb
	CPU_CFLAGS-$(CONFIG_SH2)+=-m2
	CPU_CFLAGS-$(CONFIG_SH3)+=-m3
ifeq ($(UCLIBC_HAS_FPU),y)
	CPU_CFLAGS-$(CONFIG_SH2A)+=-m2a
	CPU_CFLAGS-$(CONFIG_SH4)+=-m4
	CPU_CFLAGS-$(CONFIG_SH4A)+=-m4a
else
	CPU_CFLAGS-$(CONFIG_SH2A)+=-m2a-nofpu
	CPU_CFLAGS-$(CONFIG_SH4)+=-m4-nofpu
	CPU_CFLAGS-$(CONFIG_SH4A)+=-m4a-nofpu
endif
endif

ifeq ($(TARGET_ARCH),h8300)
	CPU_CFLAGS-$(CONFIG_H8300H) += -mh -mint32
	CPU_CFLAGS-$(CONFIG_H8S)    += -ms -mint32
endif

ifeq ($(TARGET_ARCH),cris)
	CPU_LDFLAGS-$(CONFIG_CRIS)+=-Wl,-mcrislinux
	CPU_LDFLAGS-$(CONFIG_CRISV32)+=-Wl,-mcrislinux
	CPU_CFLAGS-$(CONFIG_CRIS)+=-mlinux
	PICFLAG:=-fpic
	PIEFLAG_NAME:=-fpie
endif

ifeq ($(TARGET_ARCH),csky)
	# In csky gas implement, we use $t and $d to detect .text or literal pool.
	# So we couldn't strip them for objdump.
	STRIP_FLAGS += -K "$$"t -K "$$"d

	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)	+= -mlittle-endian
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)		+= -mbig-endian
endif

ifeq ($(TARGET_ARCH),m68k)
	# -fPIC is only supported for 68020 and above.  It is not supported
	# for 68000, 68010, or Coldfire.
	PICFLAG:=-fpic
	PIEFLAG_NAME:=-fpie
endif

ifeq ($(TARGET_ARCH),powerpc)
# PowerPC can hold 8192 entries in its GOT with -fpic which is more than
# enough. Therefore use -fpic which will reduce code size and generates
# faster code.
	PICFLAG:=-fpic
	PIEFLAG_NAME:=-fpie
	PPC_HAS_REL16:=$(shell printf "\t.text\n\taddis 11,30,_GLOBAL_OFFSET_TABLE_-.@ha\n" | $(CC) -c -x assembler -o /dev/null -  2> /dev/null && echo -n y || echo -n n)
	CPU_CFLAGS-$(PPC_HAS_REL16)+= -DHAVE_ASM_PPC_REL16
	CPU_CFLAGS-$(CONFIG_E500) += "-D__NO_MATH_INLINES"

endif

ifeq ($(TARGET_ARCH),bfin)
	SYMBOL_PREFIX=_
ifeq ($(UCLIBC_FORMAT_FDPIC_ELF),y)
	CPU_CFLAGS-y:=-mfdpic
	CPU_LDFLAGS-y += -Wl,-melf32bfinfd
	PICFLAG:=-fpic
	PIEFLAG_NAME:=-fpie
endif
ifeq ($(UCLIBC_FORMAT_SHARED_FLAT),y)
	PICFLAG := -mleaf-id-shared-library
endif
endif

ifeq ($(TARGET_ARCH),frv)
	# Using -pie causes the program to have an interpreter, which is
	# forbidden, so we must make do with -shared.  Unfortunately,
	# -shared by itself would get us global function descriptors
	# and calls through PLTs, dynamic resolution of symbols, etc,
	# which would break as well, but -Bsymbolic comes to the rescue.
	CPU_LDFLAGS-y += -Wl,-melf32frvfd -Wl,-Bsymbolic
endif

ifeq ($(strip $(TARGET_ARCH)),avr32)
       CPU_CFLAGS-$(CONFIG_AVR32_AP7)  += -march=ap
       CPU_CFLAGS-$(CONFIG_LINKRELAX)  += -mrelax
       CPU_LDFLAGS-$(CONFIG_LINKRELAX) += --relax
endif

ifeq ($(TARGET_ARCH),c6x)
	PIEFLAG:=
	CPU_CFLAGS-$(CONFIG_TMS320C64X) += -march=c64x
	CPU_CFLAGS-$(CONFIG_TMS320C64XPLUS) += -march=c64x+
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)+=-mlittle-endian
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)+=-mbig-endian
	CPU_LDFLAGS-y += $(CPU_CFLAGS)
endif

$(eval $(call check-gcc-var,$(PIEFLAG_NAME)))
PIEFLAG := $(CFLAG_$(PIEFLAG_NAME))
ifeq ($(PIEFLAG),)
PIEFLAG := $(PICFLAG)
endif
# We need to keep track of both the CC PIE flag (above) as
# well as the LD PIE flag (below) because we can't rely on
# gcc passing -pie if we used -fPIE. We need to directly use -pie
# instead of -Wl,-pie as gcc picks up the wrong startfile/endfile
$(eval $(call cache-output-var,LDPIEFLAG,$(CC) -Wl$(comma)--help 2>/dev/null | grep -q -- -pie && echo "-pie"))

# Check for --as-needed support in linker
ifndef LD_FLAG_ASNEEDED
_LD_FLAG_ASNEEDED:=$(shell $(CC) -Wl,--help 2>/dev/null | grep -- --as-needed)
ifneq ($(_LD_FLAG_ASNEEDED),)
export LD_FLAG_ASNEEDED:=--as-needed
endif
endif
ifndef LD_FLAG_NO_ASNEEDED
ifdef LD_FLAG_ASNEEDED
export LD_FLAG_NO_ASNEEDED:=--no-as-needed
endif
endif
ifndef CC_FLAG_ASNEEDED
ifdef LD_FLAG_ASNEEDED
export CC_FLAG_ASNEEDED:=-Wl,$(LD_FLAG_ASNEEDED)
endif
endif
ifndef CC_FLAG_NO_ASNEEDED
ifdef LD_FLAG_NO_ASNEEDED
export CC_FLAG_NO_ASNEEDED:=-Wl,$(LD_FLAG_NO_ASNEEDED)
endif
endif
link.asneeded = $(if $(CC_FLAG_ASNEEDED),$(if $(CC_FLAG_NO_ASNEEDED),$(CC_FLAG_ASNEEDED) $(1) $(CC_FLAG_NO_ASNEEDED)))

# Check for AS_NEEDED support in linker script (binutils>=2.16.1 has it)
ifndef ASNEEDED
export ASNEEDED:=$(shell $(CC) -Wl,--help 2>/dev/null | grep -q -- --as-needed && echo "AS_NEEDED ( $(UCLIBC_LDSO) )" || echo "$(UCLIBC_LDSO)")
endif

# Add a bunch of extra pedantic annoyingly strict checks
WARNING_FLAGS = -Wstrict-prototypes -Wstrict-aliasing -Wno-nonnull-compare
ifeq ($(EXTRA_WARNINGS),y)
WARNING_FLAGS += \
	-Wnonnull-compare \
	-Wnodeclaration-after-statement \
	-Wformat=2 \
	-Wmissing-noreturn \
	-Wmissing-format-attribute \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wnested-externs \
	-Wnonnull \
	-Wold-style-declaration \
	-Wold-style-definition \
	-Wshadow \
	-Wundef
endif
WARNING_FLAGS += $(WARNING_FLAGS-gcc-$(GCC_MAJOR_VER))
$(foreach w,$(WARNING_FLAGS),$(eval $(call check-gcc-var,$(w))))
XWARNINGS = $(call qstrip,$(WARNINGS)) $(foreach w,$(WARNING_FLAGS),$(CFLAG_$(w)))

CPU_CFLAGS=$(call qstrip,$(CPU_CFLAGS-y))

# Save the tested flag in a single variable and force it to be
# evaluated just once.  Then use that computed value.
$(eval $(call check-gcc-var,-fno-stack-protector))
SSP_DISABLE_FLAGS ?= $(CFLAG_-fno-stack-protector)
ifeq ($(UCLIBC_BUILD_SSP),y)
$(eval $(call check-gcc-var,-fno-stack-protector-all))
$(eval $(call check-gcc-var,-fstack-protector))
$(eval $(call check-gcc-var,-fstack-protector-all))
SSP_CFLAGS := $(CFLAG_-fno-stack-protector-all)
SSP_CFLAGS += $(CFLAG_-fstack-protector)
SSP_ALL_CFLAGS ?= $(CFLAG_-fstack-protector-all)
else
SSP_CFLAGS := $(SSP_DISABLE_FLAGS)
endif

# Collect all CFLAGS components
CFLAGS := $(XWARNINGS) $(CPU_CFLAGS) $(SSP_CFLAGS) \
	-nostdinc -I$(top_builddir)include \
	-I$(top_srcdir)include -include libc-symbols.h \
	-I$(top_srcdir)libc/sysdeps/linux/$(TARGET_ARCH) \
	-I$(top_srcdir)libc/sysdeps/linux \
	-I$(top_srcdir)ldso/ldso/$(TARGET_ARCH) \
	-I$(top_srcdir)ldso/include -I.

# We need this to be checked within libc-symbols.h
ifneq ($(HAVE_SHARED),y)
CFLAGS += -DSTATIC
endif

$(eval $(call check-ld-var,--warn-once))
$(eval $(call check-ld-var,--sort-common))
$(eval $(call check-ld-var,--discard-all))
LDFLAGS_NOSTRIP:=$(LDFLAG-fuse-ld) $(CPU_LDFLAGS-y) -shared \
	-Wl,--warn-common $(CFLAG_-Wl--warn-once) -Wl,-z,combreloc

ifeq ($(UCLIBC_BUILD_RELRO),y)
LDFLAGS_NOSTRIP+=-Wl,-z,relro
endif

ifeq ($(UCLIBC_BUILD_NOW),y)
LDFLAGS_NOSTRIP+=-Wl,-z,now
endif

ifeq ($(LDSO_GNU_HASH_SUPPORT),y)
# Be sure that binutils support it
$(eval $(call check-ld-var,--hash-style=gnu))
ifeq ($(LDFLAG_--hash-style=gnu),)
ifneq ($(filter-out $(clean_targets) CLEAN_% install_headers headers-y,$(MAKECMDGOALS)),)
$(error Your binutils do not support --hash-style option, while you want to use it)
endif
else
LDFLAGS_NOSTRIP += $(CFLAG_-Wl--hash-style=gnu)
endif
endif

ifeq ($(DODEBUG),y)
CFLAGS += -O0 -g3 -DDEBUG
else
CFLAGS += $(OPTIMIZATION)
CFLAGS += $(OPTIMIZATION-$(GCC_MAJOR_VER))
CFLAGS += $(OPTIMIZATION-$(GCC_MAJOR_VER).$(GCC_MINOR_VER))
$(eval $(call check-ld-var,-O2))
LDFLAGS_NOSTRIP += $(CFLAG_-Wl-O2)
endif

LDFLAGS:=$(LDFLAGS_NOSTRIP) -Wl,-z,defs

ifeq ($(DOSTRIP),y)
LDFLAGS += -Wl,-s
else
STRIPTOOL := true -Stripping_disabled
endif

ifneq ($(strip $(UCLIBC_EXTRA_CFLAGS)),"")
CFLAGS += $(call qstrip,$(UCLIBC_EXTRA_CFLAGS))
endif
ifneq ($(strip $(UCLIBC_EXTRA_LDFLAGS)),"")
LDFLAGS += $(call qstrip,$(UCLIBC_EXTRA_LDFLAGS))
endif

ifeq ($(UCLIBC_HAS_STDIO_FUTEXES),y)
CFLAGS += -D__USE_STDIO_FUTEXES__
endif

ifeq ($(UCLIBC_HAS_THREADS),y)
ifeq ($(UCLIBC_HAS_THREADS_NATIVE),y)
	PTNAME := nptl
	CFLAGS += -DHAVE_FORCED_UNWIND -D_LIBC_REENTRANT
else
ifeq ($(UCLIBC_HAS_LINUXTHREADS),y)
	PTNAME := linuxthreads
endif
endif
PTDIR := libpthread/$(PTNAME)
# set up system dependencies include dirs (NOTE: order matters!)
ifeq ($(UCLIBC_HAS_THREADS_NATIVE),y)
PTINC:= -I$(top_builddir)$(PTDIR)					\
	-I$(top_srcdir)$(PTDIR)						\
	$(if $(TARGET_SUBARCH),-I$(top_srcdir)$(PTDIR)/sysdeps/unix/sysv/linux/$(TARGET_ARCH)/$(TARGET_SUBARCH)) \
	-I$(top_srcdir)$(PTDIR)/sysdeps/unix/sysv/linux/$(TARGET_ARCH)	\
	-I$(top_builddir)$(PTDIR)/sysdeps/$(TARGET_ARCH)		\
	-I$(top_srcdir)$(PTDIR)/sysdeps/$(TARGET_ARCH)			\
	-I$(top_builddir)$(PTDIR)/sysdeps/unix/sysv/linux		\
	-I$(top_srcdir)$(PTDIR)/sysdeps/unix/sysv/linux			\
	-I$(top_srcdir)$(PTDIR)/sysdeps/pthread				\
	-I$(top_srcdir)$(PTDIR)/sysdeps/pthread/bits			\
	-I$(top_srcdir)$(PTDIR)/sysdeps/generic
#
# Test for TLS if NPTL support was selected.
#
GCC_HAS_TLS=$(shell \
	echo "extern __thread int foo;" | $(CC) -o /dev/null -S -xc - 2>&1)
ifneq ($(GCC_HAS_TLS),)
gcc_tls_test_fail:
	@echo "####";
	@echo "#### Your compiler does not support TLS and you are trying to build uClibc-ng";
	@echo "#### with NPTL support. Upgrade your binutils and gcc to versions which";
	@echo "#### support TLS for your architecture. Do not contact uClibc-ng maintainers";
	@echo "#### about this problem.";
	@echo "####";
	@echo "#### Exiting...";
	@echo "####";
	@exit 1;
endif
else
PTINC := \
	-I$(top_srcdir)$(PTDIR)/sysdeps/unix/sysv/linux/$(TARGET_ARCH) \
	-I$(top_srcdir)$(PTDIR)/sysdeps/$(TARGET_ARCH) \
	-I$(top_srcdir)$(PTDIR)/sysdeps/unix/sysv/linux \
	-I$(top_srcdir)$(PTDIR)/sysdeps/pthread \
	-I$(top_srcdir)$(PTDIR) \
	-I$(top_srcdir)libpthread
endif
CFLAGS+=$(PTINC)
else
	PTNAME :=
	PTINC  :=
endif
CFLAGS += -I$(top_srcdir)libc/sysdeps/linux/common

#CFLAGS += -iwithprefix include-fixed -iwithprefix include
$(eval $(call cache-output-var,CC_IPREFIX,$(CC) -print-file-name=include))
CC_INC := -isystem $(dir $(CC_IPREFIX))include-fixed -isystem $(CC_IPREFIX)
CFLAGS += $(CC_INC)

CFLAGS += -I$(KERNEL_HEADERS)

ifneq ($(DOASSERTS),y)
CFLAGS+=-DNDEBUG
endif

ifeq ($(SYMBOL_PREFIX),_)
CFLAGS+=-D__UCLIBC_UNDERSCORES__
endif

# Keep the check_as from being needlessly executed
ifeq ($(UCLIBC_BUILD_NOEXECSTACK),y)
$(eval $(call check-as-var,--noexecstack))
endif
ASFLAGS += $(ASFLAG_--noexecstack)

LIBGCC_CFLAGS ?= $(CFLAGS) $(CPU_CFLAGS-y)
$(eval $(call cache-output-var,LIBGCC_A,$(CC) $(LIBGCC_CFLAGS) -print-libgcc-file-name))
$(eval $(call cache-output-var,LIBGCC_EH,$(CC) $(LIBGCC_CFLAGS) -print-file-name=libgcc_eh.a))
# with -O0 we (e.g. lockf) might end up with references to
# _Unwind_Resume, so pull in gcc_eh in this case..
# with a --disable-shared toolchain, libgcc_eh.a and libgcc.a are combined
# in libgcc.a, so check if the printed file exist, before adding to the commandline
LIBGCC_EH_FILE := $(shell if [ -f $(LIBGCC_EH) ]; then echo $(LIBGCC_EH); fi)
LIBGCC_DIR := $(dir $(LIBGCC_A))
LIBGCC := $(LIBGCC_A) $(if $(DODEBUG),$(LIBGCC_EH_FILE))

# moved from libpthread/linuxthreads
ifeq ($(UCLIBC_CTOR_DTOR),y)
SHARED_START_FILES:=$(top_builddir)lib/crti.o $(LIBGCC_DIR)crtbeginS.o
SHARED_END_FILES:=$(LIBGCC_DIR)crtendS.o $(top_builddir)lib/crtn.o
endif

LOCAL_INSTALL_PATH := $(if $(O),$(O)/)install_dir

PTHREAD_GENERATE_MANGLE ?= -n "s/^.*@@@name@@@\([^@]*\)@@@value@@@[^0-9Xxa-fA-F-]*\([0-9Xxa-fA-F-][0-9Xxa-fA-F-]*\).*@@@end@@@.*\$$/\#define \1 \2/p"
