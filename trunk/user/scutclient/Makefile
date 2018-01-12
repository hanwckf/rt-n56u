SRC_NAME = scutclient-cmake-2.4
SRC_URL = https://github.com/hanwckf/scutclient-cmake/archive/v2.4.tar.gz
THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )
	
download_test:
	( if [ ! -f $(THISDIR)/$(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	cmake \
	-DCMAKE_INSTALL_PREFIX=$(STAGEDIR) \
	-DCMAKE_TOOLCHAIN_FILE=../cross-mipsel-linux.cmake \
	. \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f $(SRC_NAME)/cmake_install.cmake
	rm -rf $(SRC_NAME)/CMakeFiles
	rm -f $(SRC_NAME)/Makefile
	rm -f $(SRC_NAME)/CMakeCache.txt
	rm -f config_done

romfs:
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/scutclient /usr/bin/bin_scutclient
	${ROMFSINST} -d $(THISDIR)/scripts/enable_scutclient_watchcat /bin/enable_scutclient_watchcat
	${ROMFSINST} -d $(THISDIR)/scripts/scutclient.sh /bin/scutclient.sh
	${ROMFSINST} -d $(THISDIR)/scripts/scutclient_watchcat.sh /bin/scutclient_watchcat.sh
