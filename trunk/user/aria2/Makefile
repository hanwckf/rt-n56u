SRC_NAME=aria2-1.17.1

SRC_ARIA_NG_NAME=aria-ng-0.4.0
SRC_ARIA_NG_URL=https://github.com/mayswind/AriaNg/releases/download/0.4.0/aria-ng-0.4.0.zip

BIN_DIR=aria2-bin

CFLAGS  += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: extract_test config_test
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
	$(MAKE) -C $(BIN_DIR)
else
	$(MAKE) -C $(SRC_NAME)
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
		tar zxf $(SRC_NAME).tar.gz; \
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
	$(MAKE) -C $(BIN_DIR) clean
	rm -rf ariaweb
	rm -f config_done
	rm -rf install

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA_WEB_CONTROL),y)
	cp -rf $(THISDIR)/ariaweb $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
	$(ROMFSINST) $(THISDIR)/$(BIN_DIR)/aria2c /usr/bin/aria2c
else
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/aria2c /usr/bin/aria2c
endif
	$(ROMFSINST) -p +x /usr/bin/aria.sh
