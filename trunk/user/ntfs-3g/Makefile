SRC_NAME=ntfsprogs-2014.2.15

CFLAGS += -O2 -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all: config_test
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
		--prefix=/usr \
		--enable-shared=no \
		--enable-static=no \
		--disable-mtab \
		--disable-debug \
		--disable-library \
		--disable-ldconfig \
		--disable-mount-helper \
		--with-fuse=internal \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
ifeq ($(CONFIG_FIRMWARE_ENABLE_FUSE),y)
ifneq ($(CONFIG_FIRMWARE_ENABLE_UFSD),y)
	$(ROMFSINST) $(SRC_NAME)/src/ntfs-3g /sbin/ntfs-3g
endif
endif
