SRC_NAME=ntfs-3g_ntfsprogs-2017.3.23
SRC_URL=https://tuxera.com/opensource/ntfs-3g_ntfsprogs-2017.3.23.tgz

CFLAGS += -O2 -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tgz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tgz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tgz; \
	fi )

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
