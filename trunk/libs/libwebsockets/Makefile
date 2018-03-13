SRC_NAME=libwebsockets-2.4.2
SRC_URL=https://github.com/warmcat/libwebsockets/archive/v2.4.2.tar.gz

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	cmake \
	-DCMAKE_INSTALL_PREFIX=$(STAGEDIR) \
	-DCMAKE_TOOLCHAIN_FILE=../cross-mipsel-linux.cmake \
	-DLWS_WITHOUT_TEST_ECHO=ON \
	-DLWS_WITHOUT_TEST_SERVER=ON \
	-DLWS_WITHOUT_TEST_PING=ON \
	-DLWS_WITHOUT_CLIENT=ON \
	-DLWS_WITHOUT_TEST_FRAGGLE=ON \
	-DLWS_WITHOUT_TEST_SERVER_EXTPOLL=ON \
	. \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -rf $(SRC_NAME)/CMakeFiles
	rm -f $(SRC_NAME)/cmake_install.cmake
	rm -f $(SRC_NAME)/Makefile
	rm -f $(SRC_NAME)/CMakeCache.txt
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:
	cp -fP $(DESTDIR)/lib/libwebsockets.so* $(ROMFSDIR)/lib
