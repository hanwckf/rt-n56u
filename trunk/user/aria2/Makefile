SRC_NAME=aria2-1.17.1
BIN_DIR_NAME=aria2-1.34.0

SRC_ARIA_NG_NAME=AriaNg-1.1.3-AllInOne
SRC_ARIA_NG_URL=https://github.com/mayswind/AriaNg/releases/download/1.1.3/AriaNg-1.1.3-AllInOne.zip

CFLAGS  += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: extract_test config_test
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
	@echo $(BIN_DIR_NAME) done
else
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA_WEB_CONTROL),y)
	( if [ ! -f $(THISDIR)/$(SRC_ARIA_NG_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_ARIA_NG_NAME).zip $(SRC_ARIA_NG_URL); \
	fi )
	( if [ ! -d $(THISDIR)/ariaweb ]; then \
		unzip -qq $(SRC_ARIA_NG_NAME).zip -d ariaweb; \
	fi )
endif

extract_test:
ifneq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../disable-doc-test.patch; \
	fi )
endif

config_test:
ifneq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )
endif

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
		--with-libiconv-prefix=$(STAGEDIR) \
		PKG_CONFIG_LIBDIR="$(STAGEDIR)/lib/pkgconfig" \
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
	rm -rf ariaweb
	rm -f config_done

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA_WEB_CONTROL),y)
	cp -rf $(THISDIR)/ariaweb $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
ifeq ($(CONFIG_PRODUCT),MT7621)
	$(ROMFSINST) -S $(THISDIR)/aria2-bin/$(BIN_DIR_NAME)/mipsel_1004kc/aria2c /usr/bin/aria2c
else
	$(ROMFSINST) -S $(THISDIR)/aria2-bin/$(BIN_DIR_NAME)/mipsel_24kc/aria2c /usr/bin/aria2c
endif
else
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/aria2c /usr/bin/aria2c
endif
	$(ROMFSINST) -p +x /usr/bin/aria.sh
