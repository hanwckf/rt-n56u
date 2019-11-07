SRC_NAME=rpcbind-91973cc

CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden
LDFLAGS += -Wl,--gc-sections

all: config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

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
		--prefix=/ \
		--disable-libwrap \
		--disable-debug \
		--disable-warmstarts \
		--with-rpcuser=nobody \
		--without-nss-modules \
		--without-systemdsystemunitdir \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD); \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -rf config_done configs

romfs:
	$(ROMFSINST) $(SRC_NAME)/rpcbind /sbin/rpcbind
