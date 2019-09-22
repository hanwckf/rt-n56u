SRC_NAME=e2fsprogs-1.45.3
SRC_URL=https://mirrors.edge.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.45.3/e2fsprogs-1.45.3.tar.xz

THISDIR = $(shell pwd)

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xJf $(SRC_NAME).tar.xz; \
		patch -d $(SRC_NAME) -p1 -i ../e2fsprogs-orig.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	autoreconf -fi ; \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	./configure \
		--prefix= \
		--disable-bmap-stats \
		--disable-debugfs \
		--disable-testio-debug \
		--disable-fuse2fs \
		--disable-gcov \
		--disable-hardening \
		--disable-imager \
		--disable-resizer \
		--disable-e2initrd-helper \
		--disable-uuidd \
		--disable-mmp \
		--disable-tdb \
		--disable-rpath \
		--disable-nls \
		--disable-tls \
		--disable-libuuid \
		--disable-libblkid \
		--enable-symlink-build \
		--enable-verbose-makecmds \
		--enable-elf-shlibs \
		--enable-defrag \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/e2fsck/e2fsck /sbin/e2fsck
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/badblocks /sbin/badblocks
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/e2undo /sbin/e2undo
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/mke2fs /sbin/mke2fs
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/tune2fs /sbin/tune2fs
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/e2p/libe2p.so.2.3 /lib/libe2p.so.2.3
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/et/libcom_err.so.2.1 /lib/libcom_err.so.2.1
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/ext2fs/libext2fs.so.2.4 /lib/libext2fs.so.2.4
	$(ROMFSINST) -s libe2p.so.2.3 /lib/libe2p.so.2
	$(ROMFSINST) -s libcom_err.so.2.1 /lib/libcom_err.so.2
	$(ROMFSINST) -s libext2fs.so.2.4 /lib/libext2fs.so.2
	$(ROMFSINST) /etc_ro/e2fsck.conf
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT2),y)
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext2
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext2
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT3),y)
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext3
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext3
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT4),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/e4defrag /sbin/e4defrag
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext4
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext4
ifneq ($(CONFIG_FIRMWARE_ENABLE_EXT3),y)
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext3
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext3
endif
ifneq ($(CONFIG_FIRMWARE_ENABLE_EXT2),y)
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext2
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext2
endif
endif
