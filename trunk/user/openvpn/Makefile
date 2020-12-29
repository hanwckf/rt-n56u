SRC_NAME=openvpn-2.4.x

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

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
	autoreconf -vi ; \
	IFCONFIG=/sbin/ifconfig \
	ROUTE=/sbin/route \
	NETSTAT=/bin/netstat \
	./configure \
		--prefix=/usr \
		--disable-debug \
		--disable-plugins \
		--disable-pkcs11 \
		--disable-ofb-cfb \
		--disable-selinux \
		--disable-systemd \
		--disable-iproute2 \
		--disable-management \
		--disable-socks \
		--enable-small \
		--enable-multi \
		--enable-server \
		--enable-multihome \
		--enable-pf \
		--enable-fragment \
		--enable-http-proxy \
		--enable-def-auth \
		--enable-ssl \
		--enable-crypto \
		--enable-lzo \
		--disable-lz4 \
		--with-crypto-library=openssl \
		OPENSSL_CRYPTO_CFLAGS="-I$(STAGEDIR)/include" \
		OPENSSL_CRYPTO_LIBS="-L$(STAGEDIR)/lib -lcrypto" \
		OPENSSL_SSL_CFLAGS="-I$(STAGEDIR)/include" \
		OPENSSL_SSL_LIBS="-L$(STAGEDIR)/lib -lssl -lcrypto" \
		LZO_CFLAGS="-I$(STAGEDIR)/include" \
		LZO_LIBS="-L$(STAGEDIR)/lib -llzo2" \
		--with-sysroot=$(STAGEDIR) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(SRC_NAME)/src/openvpn/openvpn /usr/sbin/openvpn
	$(ROMFSINST) /usr/bin/openvpn-cert.sh
