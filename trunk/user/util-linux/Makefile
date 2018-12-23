SRC_NAME=util-linux-2.21.2

THISDIR = $(shell pwd)

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

BLKID_ENABLED=n

ifdef CONFIG_MMC_BLOCK
BLKID_ENABLED=y
endif
ifdef CONFIG_BLK_DEV_SD
BLKID_ENABLED=y
endif
ifdef CONFIG_MTD_UBI
BLKID_ENABLED=y
endif

all: config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) && \
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	scanf_cv_type_modifier=no \
	./configure \
		--prefix=$(STAGEDIR) \
		--without-pam \
		--without-ncurses \
		--without-slang \
		--without-selinux \
		--without-audit \
		--without-udev \
		--disable-nls \
		--disable-libmount \
		--disable-mount \
		--disable-losetup \
		--disable-fsck \
		--disable-partx \
		--disable-uuidd \
		--disable-mountpoint \
		--disable-fallocate \
		--disable-unshare \
		--disable-eject \
		--disable-agetty \
		--disable-cramfs \
		--disable-wdctl \
		--disable-switch_root \
		--disable-pivot_root \
		--disable-kill \
		--disable-utmpdump \
		--disable-rename \
		--disable-login \
		--disable-sulogin \
		--disable-su \
		--disable-schedutils \
		--disable-wall \
		--disable-use-tty-group \
		--disable-makeinstall-chown \
		--disable-makeinstall-setuid \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done
	rm -rf install

romfs:
	$(ROMFSINST) $(STAGEDIR)/bin/hexdump /usr/bin/hexdump
ifeq ($(BLKID_ENABLED),y)
	$(ROMFSINST) $(STAGEDIR)/sbin/blkid /sbin/blkid
	$(ROMFSINST) $(STAGEDIR)/lib/libblkid.so.1.1.0 /lib/libblkid.so.1.1.0
	$(ROMFSINST) $(STAGEDIR)/lib/libuuid.so.1.3.0 /lib/libuuid.so.1.3.0
	$(ROMFSINST) -s libblkid.so.1.1.0 /lib/libblkid.so
	$(ROMFSINST) -s libblkid.so.1.1.0 /lib/libblkid.so.1
	$(ROMFSINST) -s libuuid.so.1.3.0 /lib/libuuid.so
	$(ROMFSINST) -s libuuid.so.1.3.0 /lib/libuuid.so.1
endif
