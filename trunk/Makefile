############################################################################
#
# Makefile -- Top level uClinux makefile.
#
# Copyright (c) 2001-2004, SnapGear (www.snapgear.com)
# Copyright (c) 2001, Lineo
#

VERSIONPKG = 1.1

############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#

include .config
include versions.inc

ifeq ($(CONFIG_WITHOUT_KERNEL),y)
all: tools unpack_trx libc_only libs_only user_only romfs image
else
ifeq ($(CONFIG_FIRMWARE_TYPE_ROOTFS_IN_RAM),y)
all: tools modules libc_only libs_only user_only romfs linux image
else
all: tools linux libc_only libs_only user_only romfs image
endif
endif
############################################################################
#
# Get the core stuff worked out
#

ROOTDIR        = $(shell pwd)
HOSTCC         = gcc
ROMFSINST      = $(ROOTDIR)/tools/romfs-inst.sh
TFTPDIR        = /tftpboot
PATH          := $(PATH):$(ROOTDIR)/tools:$(ROOTDIR)/tools/go/go/bin

VERSIONSTR     = $(CONFIG_VENDOR)/$(CONFIG_PRODUCT) Version $(VERSIONPKG)

LINUXDIR       = $(CONFIG_LINUXDIR)
LINUXINCDIR    = $(ROOTDIR)/$(LINUXDIR)/include
IMAGEDIR       = $(ROOTDIR)/images
ROMFSDIR       = $(ROOTDIR)/romfs
STAGEDIR       = $(ROOTDIR)/stage
SCRIPTSDIR     = $(ROOTDIR)/config/scripts
LINUX_CONFIG   = $(ROOTDIR)/$(LINUXDIR)/.config
PROJECT_CONFIG = $(ROOTDIR)/.config
SSTRIP_TOOL    = $(if $(CONFIG_FIRMWARE_PERFORM_SSTRIP),$(ROOTDIR)/tools/sstrip/sstrip)
UNSQSHFS_TOOL  = $(ROOTDIR)/tools/mksquashfs_xz/unsquashfs
MERGE_DIR      = $(ROOTDIR)/merge
MERGE_ROOTFS   = $(MERGE_DIR)/$(CONFIG_FIRMWARE_PRODUCT_ID)/rootfs

#NUM MAKE PROCESS = CPU NUMBER IN THE SYSTEM * CPU_OVERLOAD
CPU_OVERLOAD	= 1
HOST_NCPU	= $(shell if [ -f /proc/cpuinfo ]; then n=`grep -c processor /proc/cpuinfo`; if [ $$n -gt 1 ];then expr $$n \* ${CPU_OVERLOAD}; else echo $$n; fi; else echo 1; fi)

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

ifneq ($(findstring linux-3.4,$(LINUXDIR)),)
FIRMWARE_KERNEL_VER="3.4"
CONFIG_CROSS_COMPILER_ROOT = $(CONFIG_TOOLCHAIN_DIR)/toolchain-3.4.x
CONFIG_CROSS_COMPILER_PATH = $(CONFIG_CROSS_COMPILER_ROOT)/bin
KERNEL_HEADERS_PATH = $(CONFIG_CROSS_COMPILER_ROOT)/mipsel-linux-uclibc/sysroot/usr/include
endif

ifeq (config.arch,$(wildcard config.arch))
ifeq ($(filter %_default, $(MAKECMDGOALS)),)
include config.arch
ARCH_CONFIG = $(ROOTDIR)/config.arch
export ARCH_CONFIG
endif
endif

# May use a different compiler for the kernel
KERNEL_CROSS_COMPILE ?= $(CROSS_COMPILE)
ifneq ($(SUBARCH),)
# Using UML, so make the kernel and non-kernel with different ARCHs
MAKEARCH = $(MAKE) ARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE) ARCH=$(ARCH) SUBARCH=$(SUBARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE)
else
MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE)
endif
DIRS = libc libs user

export VENDOR PRODUCT HOSTCC CONFIG_SHELL HOST_NCPU SSTRIP_TOOL
export PROJECT_CONFIG LINUX_CONFIG ROOTDIR LINUXDIR ROMFSDIR SCRIPTSDIR
export FIRMWARE_KERNEL_VER FIRMWARE_ROOTFS_VER FIRMWARE_BUILDS_VER FIRMWARE_BUILDS_REV
export VERSIONPKG VERSIONSTR ROMFSINST PATH IMAGEDIR RELFILES TFTPDIR
export KERNEL_HEADERS_PATH CONFIG_CROSS_COMPILER_ROOT CONFIG_CROSS_COMPILER_PATH
export UNSQSHFS_TOOL CONFIG_WITHOUT_KERNEL

############################################################################

.PHONY: modules
modules:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(LINUXDIR)/modules ] || mkdir $(LINUXDIR)/modules; \
		$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) modules; \
	fi

.PHONY: modules_install
modules_install:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=$(ROOTDIR)/tools/depmod.sh modules_install; \
	fi

.PHONY: modules_copy
modules_copy:
	[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules
	cp -rfa $(MERGE_ROOTFS)/lib/modules $(ROMFSDIR)/lib

.PHONY: bin_copy
bin_copy:
	for f in `cat $(MERGE_DIR)/file_list |grep -v "^#"`; do cp -fP $(MERGE_ROOTFS)/$$f $(ROMFSDIR)/$$f ; done

############################################################################
#
# normal make targets
#

.PHONY: romfs
ifeq ($(CONFIG_WITHOUT_KERNEL),y)
romfs: romfs.subdirs modules_copy bin_copy romfs.post
else
romfs: romfs.subdirs modules_install romfs.post
endif

.PHONY: romfs.subdirs
romfs.subdirs:
	for dir in vendors $(DIRS) ; do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir romfs || exit 1 ; done

.PHONY: romfs.post
romfs.post:
	-find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf ; \
	$(ROOTDIR)/tools/strip-romfs.sh ; \
	$(MAKEARCH) -C vendors romfs.post

.PHONY: image
image:
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	$(MAKEARCH) -C vendors image

.PHONY: linux
linux:
	$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) $(LINUXTARGET) || exit 1
	if [ -f $(LINUXDIR)/vmlinux ]; then \
		ln -f $(LINUXDIR)/vmlinux $(LINUXDIR)/linux ; \
	fi

.PHONY: sparse
sparse:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) C=1 $(LINUXTARGET) || exit 1

.PHONY: sparseall
sparseall:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) C=2 $(LINUXTARGET) || exit 1

.PHONY: subdirs
subdirs: libs
	for dir in $(DIRS) ; do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir || exit 1 ; done

dep:
	@if [ ! -f $(LINUXDIR)/.config ] ; then \
		echo "ERROR: you need to do a 'make config' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) modules_prepare
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) dep

.PHONY: tools
tools:
	make -C tools

.PHONY: unpack_trx
unpack_trx:
	unpack-trx.sh

clean:
	make clean -C tools
	for dir in  $(DIRS); do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir clean ; done
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) distclean
	rm -rf $(STAGEDIR)
	rm -rf $(ROMFSDIR)
	rm -rf $(IMAGEDIR)
	rm -rf $(LINUXDIR)/net/ipsec/alg/libaes $(LINUXDIR)/net/ipsec/alg/perlasm
	rm -f $(LINUXDIR)/arch/mips/ramdisk/*.gz

%_romfs:
	@case "$(@)" in \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(MAKEARCH) -C $(@:_romfs=) romfs;; \
	esac

%_only:
	@case "$(@)" in \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(MAKEARCH) -C $(@:_only=);; \
	esac

%_clean:
	@case "$(@)" in \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(MAKEARCH) -C $(@:_clean=) clean;; \
	esac

############################################################################
