SRC_NAME=aria2-1.35.0
SRC_URL=https://github.com/aria2/aria2/releases/download/release-1.35.0/aria2-1.35.0.tar.xz

SRC_ARIA_NG_NAME=AriaNg-1.1.7-AllInOne
SRC_ARIA_NG_URL=https://github.com/mayswind/AriaNg/releases/download/1.1.7/AriaNg-1.1.7-AllInOne.zip

CFLAGS  += -ffunction-sections -fdata-sections -fvisibility=hidden
CXXFLAGS += -ffunction-sections -fdata-sections -fvisibility-inlines-hidden
LDFLAGS += -Wl,--gc-sections -static-libstdc++

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA_WEB_CONTROL),y)
	( if [ ! -f $(THISDIR)/$(SRC_ARIA_NG_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_ARIA_NG_NAME).zip $(SRC_ARIA_NG_URL); \
	fi )
	( if [ ! -d $(THISDIR)/ariaweb ]; then \
		mkdir -p ariaweb; \
		unzip -qq $(SRC_ARIA_NG_NAME).zip -d ariaweb; \
	fi )
endif

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xf $(SRC_NAME).tar.xz; \
		patch -d $(SRC_NAME) -p1 -i ../disable-doc-test.patch; \
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
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/aria2c /usr/bin/aria2c
	$(ROMFSINST) -p +x /usr/bin/aria.sh

