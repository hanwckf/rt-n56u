SRC_NAME=aria2-1.15.2

CFLAGS  += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

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
		--disable-nls \
		--disable-xmltest \
		--without-libnettle \
		--without-libgmp \
		--without-libgcrypt \
		--without-libexpat \
		--without-libcares \
		--without-sqlite3 \
		--without-gnutls \
		--with-libz \
		--with-openssl \
		--with-libxml2 \
		--with-xml-prefix=$(STAGEDIR) \
		--with-libiconv-prefix=$(STAGEDIR) \
		ZLIB_CFLAGS="-I$(STAGEDIR)/include" \
		ZLIB_LIBS="-L$(STAGEDIR)/lib -lz" \
		OPENSSL_CFLAGS="-I$(STAGEDIR)/include" \
		OPENSSL_LIBS="-L$(STAGEDIR)/lib -lssl -lcrypto" \
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
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA_WEB_CONTROL),y)
	cp -rf $(THISDIR)/ariaweb $(INSTALLDIR)/www
	cd $(INSTALLDIR)/www/ariaweb && ln -sf /mnt/aria/config/configuration.js configuration.js
endif
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/aria2c /usr/bin/aria2c
	$(ROMFSINST) -p +x /usr/bin/aria.sh
