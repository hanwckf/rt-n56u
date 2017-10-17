SRC_NAME=strongswan-5.6.0
URL_PATH=https://download.strongswan.org/$(SRC_NAME).tar.bz2

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections,-L$(STAGEDIR)/lib,-rpath-link=$(THISDIR)/$(SRC_NAME)/src/libtls/.libs -lcrypto

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)
	$(MAKE) -C $(SRC_NAME) install-strip DESTDIR=$(THISDIR)/install

download_test:
	( if [ ! -f $(THISDIR)/$(SRC_NAME).tar.bz2 ]; then \
		wget -t5 --timeout=20 --no-check-certificate $(URL_PATH); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		tar xjf $(SRC_NAME).tar.bz2; \
	fi )

config_test:
	( if [ -f $(THISDIR)/config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	./configure \
		--with-sysroot=$(STAGEDIR) \
		--prefix=/usr \
		--sysconfdir=/etc/storage/strongswan \
		--disable-pki \
		--disable-fips-prf \
		--disable-gmp \
		--disable-rc2 \
		--disable-cmac \
		--disable-xcbc \
		--disable-scepclient \
		--disable-vici \
		--disable-swanctl \
		--disable-dnskey \
		--disable-pgp \
		--disable-sshkey \
		--disable-constraints \
		--disable-revocation \
		--disable-random \
		--disable-des \
		--disable-aes \
		--disable-md5 \
		--disable-sha1 \
		--disable-sha2 \
		--disable-pkcs7 \
		--disable-pkcs12 \
		--disable-hmac \
		--enable-dhcp \
		--enable-farp \
		--enable-openssl \
		--enable-eap-dynamic \
		--enable-eap-identity \
		--enable-eap-tls \
		--enable-eap-mschapv2 \
		--enable-xauth-eap \
		--enable-xauth-noauth \
		--enable-systime-fix \
		--enable-monolithic \
		--with-resolv-conf=/etc/resolv.conf \
		--with-user=admin \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	rm -f config_done
	rm -rf $(THISDIR)/install
	rm -rf $(THISDIR)/$(SRC_NAME)

romfs:
	mkdir -p $(INSTALLDIR)/usr/lib/ipsec
	mkdir -p $(INSTALLDIR)/usr/libexec/ipsec
	cp -fP $(THISDIR)/install/usr/lib/ipsec/*.so.0* $(ROMFSDIR)/usr/lib/ipsec
	$(ROMFSINST) $(THISDIR)/install/usr/sbin/ipsec /usr/sbin/ipsec
	$(ROMFSINST) $(THISDIR)/install/usr/libexec/ipsec/charon /usr/libexec/ipsec/charon
	$(ROMFSINST) $(THISDIR)/install/usr/libexec/ipsec/starter /usr/libexec/ipsec/starter
	$(ROMFSINST) $(THISDIR)/install/usr/libexec/ipsec/stroke /usr/libexec/ipsec/stroke
	$(ROMFSINST) $(THISDIR)/install/usr/libexec/ipsec/_updown /usr/libexec/ipsec/_updown
