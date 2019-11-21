SRC_NAME=mbedtls-2.7.12
SRC_URL=https://tls.mbed.org/download/mbedtls-2.7.12-gpl.tgz

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build

download_test:
	( if [ ! -f $(SRC_NAME).tgz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tgz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tgz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
	cmake \
	-DCMAKE_INSTALL_PREFIX=$(STAGEDIR) \
	-DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake \
	-DUSE_SHARED_MBEDTLS_LIBRARY=On \
	-DUSE_STATIC_MBEDTLS_LIBRARY=Off \
	-DENABLE_TESTING=Off \
	-DENABLE_PROGRAMS=Off \
	-DLINK_WITH_PTHREAD=On \
	-DENABLE_ZLIB_SUPPORT=On \
	.. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME)/build install DESTDIR=""

romfs:
	cp -fP $(DESTDIR)/lib/libmbedcrypto.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libmbedtls.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libmbedx509.so* $(ROMFSDIR)/lib
