SRC_NAME=openssl-1.0.2u
SRC_URL=https://www.openssl.org/source/old/1.0.2/$(SRC_NAME).tar.gz

# openssl use CROSS_COMPILE prefix
CC=gcc $(CPUFLAGS)

COPTS=-ffunction-sections -fdata-sections -I$(DESTDIR)/include
COPTS+=$(if $(CONFIG_IPV6),-DOPENSSL_USE_IPV6=1,)
COPTS+=-DOPENSSL_SMALL_FOOTPRINT

LOPTS=-Wl,--gc-sections -L$(DESTDIR)/lib

OPENSSL_APPS =

OPENSSL_CIPHERS = no-capieng no-cms no-gmp no-jpake no-rfc3779 no-seed \
                  no-ec2m no-err no-threads no-idea no-rc5 no-ripemd no-rmd160 \
                  no-static-engine no-md2 no-sse2 no-dso no-ts no-sha0 no-mdc2 no-krb5 \
                  no-smime no-ans1 no-cast no-whirlpool no-sctp no-srp no-ssl2 no-ssl3

ifdef CONFIG_CRYPTO_DEV_LINUX
COPTS+=-DHAVE_CRYPTODEV
else
OPENSSL_CIPHERS += no-hw
endif

ifneq ($(CONFIG_FIRMWARE_INCLUDE_SSWAN),y)
OPENSSL_CIPHERS += no-rc2
endif

ifneq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EC),y)
OPENSSL_CIPHERS += no-ec
endif

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME) depend
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xzf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../openssl-orig.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./Configure \
		linux-mipsel \
		shared \
		--prefix=/ \
		--install_prefix=$(STAGEDIR) \
		--openssldir=/etc/ssl \
		$(COPTS) \
		$(LOPTS) \
	)
	( cd $(SRC_NAME) ; \
	./Configure \
		linux-mipsel \
		shared \
		--prefix=/ \
		--install_prefix=$(STAGEDIR) \
		--openssldir=/etc/ssl \
		no-symlinks \
		$(COPTS) \
		$(LOPTS) \
		$(OPENSSL_APPS) \
		$(OPENSSL_CIPHERS) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -rf $(SRC_NAME)/include/*
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR="$(STAGEDIR)"
	rm -f $(DESTDIR)/lib/libssl.a
	rm -f $(DESTDIR)/lib/libcrypto.a

romfs:
	cp -fP $(DESTDIR)/lib/libssl.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libcrypto.so* $(ROMFSDIR)/lib
	$(ROMFSINST) /etc_ro/openssl.cnf
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EXE),y)
	$(ROMFSINST) $(SRC_NAME)/apps/openssl /usr/bin/openssl
endif
