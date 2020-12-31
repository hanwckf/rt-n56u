SRC_NAME=openssl-1.1.1i
SRC_URL=https://www.openssl.org/source/$(SRC_NAME).tar.gz

# openssl use CROSS_COMPILE prefix
CC = gcc
AR = ar
RANLIB = ranlib
CXX = g++

COPTS = $(CPUFLAGS) -O3 $(filter-out -O%, $(CFLAGS))
COPTS += $(if $(CONFIG_IPV6),-DOPENSSL_USE_IPV6=1,)
COPTS += -DOPENSSL_PREFER_CHACHA_OVER_GCM

CFLAGS := $(COPTS)
CXXFLAGS := $(COPTS)

OPENSSL_CIPHERS = no-capieng no-cms no-rfc3779 no-seed \
  no-ec2m no-err no-threads no-idea no-rc5 no-rmd160 \
  no-static-engine no-md2 no-sse2 no-dso no-ts no-mdc2 \
  no-cast no-whirlpool no-sctp no-srp no-ssl2 no-ssl3 \
  no-async no-blake2 no-hw-padlock no-aria no-hw \
  no-devcryptoeng no-ssl3-method no-tests no-sm2 \
  no-sm3 no-sm4 no-afalgeng no-comp

ifneq ($(CONFIG_FIRMWARE_INCLUDE_SSWAN),y)
OPENSSL_CIPHERS += no-rc2
endif

ifneq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EC),y)
OPENSSL_CIPHERS += no-ec
endif

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 < $(SRC_NAME).patch; \
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
		linux-mips32 \
		shared \
		--prefix=$(STAGEDIR) \
		--openssldir=/etc/ssl \
		$(OPENSSL_CIPHERS) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install_sw DESTDIR=""
	$(MAKE) -C $(SRC_NAME) install_ssldirs DESTDIR="$(STAGEDIR)"
	rm -f $(DESTDIR)/lib/libssl.a
	rm -f $(DESTDIR)/lib/libcrypto.a

romfs:
	cp -fP $(DESTDIR)/lib/libssl.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libcrypto.so* $(ROMFSDIR)/lib
	$(ROMFSINST) /etc_ro/openssl.cnf
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSL_EXE),y)
	$(ROMFSINST) $(SRC_NAME)/apps/openssl /usr/bin/openssl
endif

