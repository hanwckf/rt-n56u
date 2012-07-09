SRC_NAME=transmission-2.60

# fix __cxa_atexit for libutp
CXXFLAGS += -fno-use-cxa-atexit

CFLAGS   += -ffunction-sections -fdata-sections -Wl,--gc-sections -D_LARGEFILE64_SOURCE
LDFLAGS  += -Wl,--gc-sections

THISDIR = $(shell pwd)

all:	config_test
	$(MAKE) -C $(SRC_NAME) && \
	$(MAKE) -C $(SRC_NAME) install DESTDIR=$(THISDIR)/install

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
	./configure \
		--prefix=/usr \
		--enable-daemon \
		--enable-utp \
		--with-inotify \
		--without-gtk \
		--disable-nls \
		--disable-cli \
		--disable-mac \
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
	cp -rf $(THISDIR)/install/usr/share/transmission $(INSTALLDIR)/usr/share
	$(ROMFSINST) $(THISDIR)/install/usr/bin/transmission-daemon /usr/bin/transmission-daemon
	$(ROMFSINST) -p +x /usr/bin/transmission.sh
