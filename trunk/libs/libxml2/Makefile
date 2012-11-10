SRC_NAME=libxml2-2.7.8

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
		--prefix=$(STAGEDIR) \
		--enable-static \
		--disable-shared \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--without-catalog \
		--without-debug \
		--without-docbook \
		--without-ftp \
		--without-http \
		--without-legacy \
		--without-pattern \
		--without-push \
		--without-python \
		--without-readline \
		--without-regexps \
		--without-iconv \
		--without-iso8859x \
		--with-c14n \
		--with-html \
		--with-output \
		--with-reader \
		--with-sax1 \
		--with-schemas \
		--with-threads \
		--with-tree \
		--with-valid \
		--with-writer \
		--with-xinclude \
		--with-xpath \
		--with-xptr \
		--with-zlib=$(STAGEDIR) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:


