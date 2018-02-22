SRC_NAME = dogcom-1.6.2-pdv
SRC_URL = https://github.com/hanwckf/dogcom/archive/v1.6.2-pdv.tar.gz

THISDIR = $(shell pwd)

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME) CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc
	
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
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/dogcom /usr/bin/dogcom
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/sample-p.conf /etc_ro/dogcom/sample-p.conf
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/sample-d.conf /etc_ro/dogcom/sample-d.conf
