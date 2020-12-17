SRC_NAME = wireguard-tools-1.0.20200827
SRC_URL = https://git.zx2c4.com/wireguard-tools/snapshot/wireguard-tools-1.0.20200827.tar.xz
THISDIR = $(shell pwd)

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME)/src WITH_BASHCOMPLETION=no WITH_WGQUICK=no WITH_SYSTEMDUNITS=no wg

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.xz; \
	fi )

clean:
	if [ -f $(SRC_NAME)/src/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME)/src clean ; \
	fi

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/wg /usr/bin/wg

