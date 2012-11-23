SRC_NAME=e2fsprogs-1.42.6

THISDIR = $(shell pwd)

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

ifeq ($(KERNEL3X),y)
E4DEFRAG=--enable-defrag
else
E4DEFRAG=--disable-defrag
endif

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	./configure \
		--prefix= \
		--disable-compression \
		--disable-debugfs \
		--disable-testio-debug \
		--disable-imager \
		--disable-resizer \
		--disable-e2initrd-helper \
		--disable-uuidd \
		--disable-nls \
		--disable-tls \
		--disable-libuuid \
		--disable-libblkid \
		--disable-libquota \
		--enable-symlink-build \
		--enable-verbose-makecmds \
		--enable-elf-shlibs \
		$(E4DEFRAG) \
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
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/mke2fs /sbin/mke2fs
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/tune2fs /sbin/tune2fs
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/e2p/libe2p.so.2.3 /lib/libe2p.so.2.3
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/et/libcom_err.so.2.1 /lib/libcom_err.so.2.1
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/lib/ext2fs/libext2fs.so.2.4 /lib/libext2fs.so.2.4
	$(ROMFSINST) -s libe2p.so.2.3 /lib/libe2p.so.2
	$(ROMFSINST) -s libcom_err.so.2.1 /lib/libcom_err.so.2
	$(ROMFSINST) -s libext2fs.so.2.4 /lib/libext2fs.so.2
	$(ROMFSINST) /etc_ro/e2fsck.conf
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext2
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext3
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext2
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext3
ifeq ($(KERNEL3X),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/misc/e4defrag /sbin/e4defrag
	$(ROMFSINST) -s mke2fs /sbin/mkfs.ext4
	$(ROMFSINST) -s e2fsck /sbin/fsck.ext4
endif
