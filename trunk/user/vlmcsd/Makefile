THISDIR = $(shell pwd)
SRC_NAME = vlmcsd-1112-pdv
SRC_URL = https://github.com/hanwckf/vlmcsd/archive/v1112-pdv.zip

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME) vlmcsd CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc
	
download_test: 
	( if [ ! -f $(THISDIR)/$(SRC_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).zip $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		unzip -qq $(SRC_NAME).zip; \
	fi )

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/bin/vlmcsd /usr/bin/vlmcsd
	$(ROMFSINST) -p +x $(THISDIR)/vlmcsd.sh /usr/bin/vlmcsd.sh

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
