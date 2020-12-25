SRC_NAME = 3proxy-0.9.3
SRC_URL = https://github.com/z3APA3A/3proxy/releases/download/0.9.3/3proxy-0.9.3.tar.gz
THISDIR = $(shell pwd)

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME) -f Makefile.Linux

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../$(SRC_NAME).patch; \
	fi )

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/bin/3proxy /usr/bin/3proxy
	$(ROMFSINST) -S $(THISDIR)/$(SRC_NAME)/bin/TransparentPlugin.ld.so /usr/lib/TransparentPlugin.ld.so

