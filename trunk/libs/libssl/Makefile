SRC_NAME=openssl-1.0.1c

# openssl use CROSS_COMPILE prefix
CC=gcc $(CPUFLAGS)

COPTS=-ffunction-sections -fdata-sections -Wl,--gc-sections -I$(DESTDIR)/include
COPTS+=$(if $(CONFIG_IPV6),-DUSE_IPV6,)
COPTS+=-DOPENSSL_SMALL_FOOTPRINT

LOPTS=-Wl,--gc-sections -L$(DESTDIR)/lib

OPENSSL_CIPHERS = no-camellia no-capieng no-cms no-gmp no-jpake no-krb5 no-mdc2 no-rfc3779 no-seed \
                  no-ec no-ecdh no-ec2m no-ecdsa no-err no-threads no-idea no-rc2 no-rc5 no-sha0 \
                  no-engine no-hw no-md2 no-md4 no-sse2 no-dso no-ts no-aes192 no-ripemd no-rmd160 \
                  no-smime no-ans1 no-cast no-whirlpool no-sctp no-srp no-jpake no-rfc3779 no-ssl2

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
	./Configure \
		linux-mipsel \
		shared \
		--prefix=$(STAGEDIR) \
		--install_prefix= \
		$(COPTS) \
		$(LOPTS) \
		$(OPENSSL_CIPHERS) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) clean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:
	cp -fP $(DESTDIR)/lib/libssl.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libcrypto.so* $(ROMFSDIR)/lib
