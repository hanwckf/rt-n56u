# use host target
CC        = gcc
GCC       = gcc
CXX       = g++
AS        = as
AR        = ar
LD        = ld
NM        = nm
OBJCOPY   = objcopy
RANLIB    = ranlib
STRIP     = strip
STRIPTOOL = $(STRIP)

CFLAGS    = -Wall -O2
LDFLAGS   =

-include $(LINUX_CONFIG)

all:
	make -C mkimage
	make -C sstrip
	make -C lzma
ifeq ($(CONFIG_RT2880_ROOTFS_IN_FLASH),y)
	make -C mksquashfs_xz
endif

clean:
	make -C sstrip clean
	make -C mkimage clean
	make -C lzma clean
	make -C mksquashfs_xz clean
