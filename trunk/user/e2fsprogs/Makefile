SRC_NAME="e2fsprogs-1.41.14"

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

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
	./configure \
		--disable-compression \
		--disable-profile \
		--disable-checker \
		--disable-fsck \
		--disable-debugfs \
		--disable-testio-debug \
		--disable-imager \
		--disable-resizer \
		--disable-tls \
		--disable-nls \
		--disable-uuidd \
		--disable-e2initrd-helper \
		--enable-symlink-build \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -f e2fsck
	rm -f mke2fs
	rm -f blkid
	rm -f config_done

romfs:
	cp $(SRC_NAME)/e2fsck/e2fsck .
	cp $(SRC_NAME)/misc/mke2fs .
	cp $(SRC_NAME)/misc/blkid .
	$(STRIP) e2fsck
	$(STRIP) mke2fs
	$(STRIP) blkid
	$(ROMFSINST) /sbin/e2fsck
	$(ROMFSINST) /sbin/mke2fs
	$(ROMFSINST) /sbin/blkid
	$(ROMFSINST) /etc_ro/mke2fs.conf
	$(ROMFSINST) /etc_ro/e2fsck.conf
	cd $(INSTALLDIR)/sbin && ln -sf mke2fs mkfs.ext2
	cd $(INSTALLDIR)/sbin && ln -sf mke2fs mkfs.ext3
	cd $(INSTALLDIR)/sbin && ln -sf e2fsck fsck.ext2
	cd $(INSTALLDIR)/sbin && ln -sf e2fsck fsck.ext3
ifeq ($(KERNEL3X),y)
	cd $(INSTALLDIR)/sbin && ln -sf mke2fs mkfs.ext4
	cd $(INSTALLDIR)/sbin && ln -sf e2fsck fsck.ext4
endif
