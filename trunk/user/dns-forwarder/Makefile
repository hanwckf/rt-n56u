SRC_NAME = dns-forwarder-1.2.1
THISDIR = $(shell pwd)

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
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/dns-forwarder /usr/bin/dns-forwarder
	$(ROMFSINST) -p +x $(THISDIR)/dns-forwarder.sh /usr/bin/dns-forwarder.sh
