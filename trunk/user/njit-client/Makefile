SRC_NAME = njit8021xclient-1.3.1-pdv
SRC_URL = https://github.com/hanwckf/njit8021xclient/archive/v1.3.1-pdv.tar.gz
THISDIR = $(shell pwd)

CFLAGS += -Wno-unused-value

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)/src

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../0001-fix-njit_client-build-error.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME)/src ; \
	autoreconf -fi ; \
	ac_cv_path_PCAP_CONFIG=$(STAGEDIR)/bin/pcap-config \
	./configure \
		--prefix=/usr \
		PKG_CONFIG_LIBDIR=$(STAGEDIR)/lib/pkgconfig \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/src/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME)/src distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/client /usr/bin/njit-client
