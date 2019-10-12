# Kbuild for the AVM NTFS filesystem.
#
# vim: set ft=make noet:

PWD  := $(shell pwd)
KDIR := $(ROOTDIR)/$(CONFIG_LINUXDIR)

MODULE_CONFIG := CONFIG_ANTFS_FS=m
MODULE_CONFIG += CONFIG_ANTFS_SYMLINKS=y

LIBNTFS_SRC := libntfs-3g

ANTFS_LOGLEVEL_DEFAULT ?= ANTFS_LOGLEVEL_ERR
ANTFS_VERSION ?= 07.11

ccflags-y += -DANTFS_LOGLEVEL_DEFAULT=$(ANTFS_LOGLEVEL_DEFAULT)
ccflags-y += -DANTFS_VERSION='"$(ANTFS_VERSION)"'

ccflags-y += -I$(src)
ccflags-y += -I$(src)/include
#ccflags-$(CONFIG_ANTFS_EXTRA_WARNINGS) += -Wextra -Wshadow -Werror -Wno-error=shadow

obj-$(CONFIG_ANTFS_FS) += antfs.o

antfs-y += dir.o
antfs-y += file.o
antfs-y += inode.o
antfs-y += super.o
antfs-y += $(LIBNTFS_SRC)/attrib.o
antfs-y += $(LIBNTFS_SRC)/attrlist.o
antfs-y += $(LIBNTFS_SRC)/bootsect.o
antfs-y += $(LIBNTFS_SRC)/collate.o
antfs-y += $(LIBNTFS_SRC)/compress.o
antfs-y += $(LIBNTFS_SRC)/device.o
antfs-y += $(LIBNTFS_SRC)/dir.o
antfs-y += $(LIBNTFS_SRC)/index.o
antfs-y += $(LIBNTFS_SRC)/inode.o
antfs-y += $(LIBNTFS_SRC)/lcnalloc.o
antfs-y += $(LIBNTFS_SRC)/linux_io.o
antfs-y += $(LIBNTFS_SRC)/logfile.o
antfs-y += $(LIBNTFS_SRC)/mft.o
antfs-y += $(LIBNTFS_SRC)/misc.o
antfs-y += $(LIBNTFS_SRC)/mst.o
antfs-y += $(LIBNTFS_SRC)/object_id.o
antfs-y += $(LIBNTFS_SRC)/reparse.o
antfs-y += $(LIBNTFS_SRC)/runlist.o
antfs-y += $(LIBNTFS_SRC)/unistr.o
antfs-y += $(LIBNTFS_SRC)/volume.o

all:
	$(MAKEARCH_KERNEL) -C $(KDIR) M=$(PWD) modules $(MODULE_CONFIG)

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
	rm -rf $(LIBNTFS_SRC)/*.o $(LIBNTFS_SRC)/.*.cmd
