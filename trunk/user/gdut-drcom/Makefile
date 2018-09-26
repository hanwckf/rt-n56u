SRC_NAME = gdut-drcom-1.6.8
SRC_URL = https://github.com/chenhaowen01/gdut-drcom/archive/1.6.8.tar.gz

THISDIR = $(shell pwd)

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME)/src CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
	fi )

clean:
	( if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi )

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/gdut-drcom /usr/bin/gdut-drcom
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/src/gdut-drcom.conf /etc_ro/gdut-drcom/gdut-drcom.conf
