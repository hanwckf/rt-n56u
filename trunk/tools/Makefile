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
SSTRIP_NAME = ELFkickers-3.1

CFLAGS    = -Wall -O2
LDFLAGS   =

-include $(LINUX_CONFIG)

all:
	make -C mkimage
	mkdir -p sstrip
	make -C $(SSTRIP_NAME)
	make -C lzma
ifeq ($(CONFIG_RT2880_ROOTFS_IN_FLASH),y)
	make -C mksquashfs_xz
endif

clean:
	if [ -f $(SSTRIP_NAME)/Makefile ] ; then \
		make -C $(SSTRIP_NAME) clean ; \
	fi ;
	rm -rf sstrip
	make -C mkimage clean
	make -C lzma clean
	make -C mksquashfs_xz clean
