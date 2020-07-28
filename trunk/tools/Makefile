THISDIR = $(shell pwd)
go_version = 1.14
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

all:go_build
	make -C mkimage
	mkdir -p sstrip
	make -C $(SSTRIP_NAME)
	make -C lzma
ifeq ($(CONFIG_RT2880_ROOTFS_IN_FLASH),y)
	make -C mksquashfs_xz
endif

go_build:
	mkdir -p $(THISDIR)/go
	( if [ ! -f $(THISDIR)/go/go$(go_version).linux-amd64.tar.gz ]; then \
	curl --create-dirs -L https://studygolang.com/dl/golang/go$(go_version).linux-amd64.tar.gz -o $(THISDIR)/go/go$(go_version).linux-amd64.tar.gz ; \
	fi )
	( if [ ! -d $(THISDIR)/go/go ]; then \
	tar zxfv $(THISDIR)/go/go1.14.linux-amd64.tar.gz -C go ; \
	fi )

clean:
	if [ -f $(SSTRIP_NAME)/Makefile ] ; then \
		make -C $(SSTRIP_NAME) clean ; \
	fi ;
	rm -rf sstrip
	make -C mkimage clean
	make -C lzma clean
	make -C mksquashfs_xz clean
	rm -rf $(THISDIR)/go/go
