SRC_NAME = njit8021xclient-master
THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)/src

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip $(SRC_NAME).zip; \
		patch -p1 -d $(SRC_NAME) < $(SRC_NAME).patch; \
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
