SRC_NAME=curl-7.21.7

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	curl_cv_writable_argv=yes \
	./configure \
		--prefix=/ \
		$(if $(CONFIG_LIB_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(CONFIG_LIB_BUILD_SHARED),--enable-shared,--disable-shared) \
		$(if $(CONFIG_FIRMWARE_ENABLE_IPV6),--enable-ipv6,--disable-ipv6) \
		--disable-gopher \
		--disable-ldap \
		--disable-rtsp \
		--disable-telnet \
		--disable-tftp \
		--disable-smtp \
		--disable-pop3 \
		--disable-imap \
		--disable-dict \
		--disable-ares \
		--disable-debug \
		--disable-curldebug \
		--disable-manual \
		--disable-verbose \
		--without-gnutls \
		--without-ca-bundle \
		--without-libidn \
		--with-random=/dev/urandom \
		--with-ssl=$(STAGEDIR) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	if [ $(CONFIG_LIB_BUILD_SHARED) = "y" ] ; then \
		cp -fP $(DESTDIR)/lib/libcurl.so* $(ROMFSDIR)/lib ; \
	fi

