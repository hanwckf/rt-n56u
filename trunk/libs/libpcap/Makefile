SRC_NAME=libpcap-1.1.1

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
	export ac_cv_linux_vers=2 ; \
	./configure \
		--prefix=/ \
		$(if $(CONFIG_LIB_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(CONFIG_LIB_BUILD_SHARED),--enable-shared,--disable-shared) \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--disable-yydebug \
		--with-pcap=linux \
		--without-dag \
		--without-septel \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install ; \
	if [ -z $(CONFIG_LIB_BUILD_SHARED) ] ; then \
		rm -f $(DESTDIR)/lib/libpcap.so* ; \
	fi

romfs:
	if [ $(CONFIG_LIB_BUILD_SHARED) = "y" ] ; then \
		cp -fP $(DESTDIR)/lib/libpcap.so* $(ROMFSDIR)/lib ; \
	fi
