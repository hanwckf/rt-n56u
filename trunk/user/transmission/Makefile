SRC_NAME = transmission-3.00
SRC_URL = https://github.com/transmission/transmission/releases/download/3.00/transmission-3.00.tar.xz
CFLAGS   += -O2 -ffunction-sections -fdata-sections -fvisibility=hidden -D_LARGEFILE64_SOURCE -D_LFS64_LARGEFILE -Wno-implicit-fallthrough
LDFLAGS  += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) && \
	$(MAKE) -C $(SRC_NAME) install DESTDIR=$(THISDIR)/install

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.xz; \
		patch -p1 -d $(SRC_NAME) < $(SRC_NAME).patch; \
		mkdir -p $(SRC_NAME)/third-party/miniupnpc/include/miniupnp ; \
		cp -r $(SRC_NAME)/third-party/miniupnpc/*.h $(SRC_NAME)/third-party/miniupnpc/include/miniupnp ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	HAVE_CXX=yes \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	./autogen.sh \
		--prefix=/usr \
		--enable-daemon \
		--enable-lightweight \
		--enable-utp \
		--disable-nls \
		--disable-cli \
		--disable-mac \
		--without-gtk \
		--without-systemd \
		--with-inotify \
		--with-crypto=openssl \
		--with-zlib=$(STAGEDIR) \
		OPENSSL_CFLAGS="-I$(STAGEDIR)/include" \
		OPENSSL_LIBS="-L$(STAGEDIR)/lib -lssl -lcrypto" \
		LIBCURL_CFLAGS="-I$(STAGEDIR)/include" \
		LIBCURL_LIBS="-L$(STAGEDIR)/lib -lcurl -Wl,-rpath-link=$(STAGEDIR)/lib" \
		LIBEVENT_CFLAGS="-I$(STAGEDIR)/include" \
		LIBEVENT_LIBS="-L$(STAGEDIR)/lib -levent" \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done
	rm -rf install

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION_WEB_CONTROL),y)
	mkdir -p $(INSTALLDIR)/usr/share/transmission
	cp -rf $(THISDIR)/transmission-control/web $(INSTALLDIR)/usr/share/transmission
else
	cp -rf $(THISDIR)/install/usr/share/transmission $(INSTALLDIR)/usr/share
	rm -f $(INSTALLDIR)/usr/share/transmission/web/LICENSE
endif
	$(ROMFSINST) $(THISDIR)/install/usr/bin/transmission-daemon /usr/bin/transmission-daemon
	$(ROMFSINST) -p +x /usr/bin/transmission.sh
