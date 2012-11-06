SRC_NAME=aria2-1.15.2

CFLAGS  += -ffunction-sections -fdata-sections
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
		--without-gnutls \
		--without-libnettle \
		--without-libgmp \
		--without-libgcrypt \
		--without-libexpat \
		--without-libcares \
		--without-sqlite3 \
		--without-gnutls \
		--with-libz \
		--with-libz-prefix=$(STAGEDIR) \
		--with-libxml2 \
		--with-xml-prefix=$(STAGEDIR) \
		--with-openssl \
		--with-openssl-prefix=$(STAGEDIR) \
		--with-libiconv-prefix=$(STAGEDIR) \
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
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/aria2c /usr/bin/aria2c
	$(ROMFSINST) -p +x /usr/bin/aria.sh
