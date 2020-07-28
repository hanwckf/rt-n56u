SRC_NAME=openssl-1.1.1

# openssl use CROSS_COMPILE prefix
CC=gcc $(CPUFLAGS)
CXX=g++ $(CPUFLAGS)
AR=ar
RANLIB=ranlib

COPTS=$(CFLAGS) -O3
COPTS+=$(if $(CONFIG_IPV6),-DOPENSSL_USE_IPV6=1,)
COPTS+=-DOPENSSL_PREFER_CHACHA_OVER_GCM

LOPTS=-Wl,--gc-sections -L$(DESTDIR)/lib

OPENSSL_OPTIONS = no-capieng no-cms no-rfc3779 no-seed no-async \
                  no-ec2m no-err no-idea no-rc5 no-rmd160 no-blake2 \
                  no-static-engine no-hw-padlock no-md2 no-sse2 no-dso no-ts no-mdc2 \
                  no-cast no-whirlpool no-sctp no-srp no-ssl3 \
                  no-aria no-sm2 no-sm3 no-sm4

ifneq ($(CONFIG_FIRMWARE_INCLUDE_TOR),y)
OPENSSL_OPTIONS += no-threads
endif

#ifdef CONFIG_CRYPTO_DEV_LINUX
#OPENSSL_OPTIONS += enable-devcryptoeng
#else
OPENSSL_OPTIONS += no-hw no-devcryptoeng
ifneq ($(CONFIG_FIRMWARE_INCLUDE_TOR),y)
OPENSSL_OPTIONS += no-engine
endif
#endif

ifneq ($(CONFIG_FIRMWARE_INCLUDE_SSWAN),y)
OPENSSL_OPTIONS += no-rc2
endif

ifeq (,$(filter y,$(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EC) $(CONFIG_FIRMWARE_INCLUDE_TOR)))
OPENSSL_OPTIONS += no-ec
endif

all: config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./Configure \
		linux-mips32 \
		shared \
		--prefix=/ \
		--openssldir=/etc/ssl \
		$(COPTS) \
		$(LOPTS) \
		$(OPENSSL_OPTIONS) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR="$(STAGEDIR)"
	$(MAKE) -C $(SRC_NAME) install_ssldirs DESTDIR="$(STAGEDIR)"
	# remove unneeded
	rm -f $(DESTDIR)/lib/libssl.a
	rm -f $(DESTDIR)/lib/libcrypto.a
	# fix pkgconfig
	sed -i 's,^prefix=.*,prefix=$(STAGEDIR),g' $(STAGEDIR)/lib/pkgconfig/libcrypto.pc
	sed -i 's,^prefix=.*,prefix=$(STAGEDIR),g' $(STAGEDIR)/lib/pkgconfig/libssl.pc
	sed -i 's,^prefix=.*,prefix=$(STAGEDIR),g' $(STAGEDIR)/lib/pkgconfig/openssl.pc

romfs:
	cp -fP $(DESTDIR)/lib/libssl.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libcrypto.so* $(ROMFSDIR)/lib
	$(ROMFSINST) /etc_ro/openssl.cnf
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EXE),y)
	$(ROMFSINST) $(SRC_NAME)/apps/openssl /usr/bin/openssl
endif
