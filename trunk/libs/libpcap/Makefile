SRC_NAME=libpcap-1.4.0

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
	ac_cv_netfilter_can_compile=no \
	./configure \
		--prefix=$(STAGEDIR) \
		--enable-static \
		--disable-shared \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--disable-yydebug \
		--disable-can \
		--disable-usb \
		--disable-canusb \
		--disable-bluetooth \
		--with-pcap=linux \
		--without-flex \
		--without-bison \
		--without-dag \
		--without-septel \
		--without-libnl \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR="" ;
	rm -f $(DESTDIR)/lib/libpcap.so* ;

romfs:

