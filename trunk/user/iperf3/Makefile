SRC_NAME = iperf-3.1.3
SRC_URL = https://iperf.fr/download/source/iperf-3.1.3-source.tar.gz
THISDIR = $(shell pwd)

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
	sed -i 's/iperf3_profile_CFLAGS = -pg -g//' src/Makefile.in ; \
	sed -i 's/iperf3_profile_LDFLAGS = -pg -g//' src/Makefile.in ; \
	./configure \
		--prefix=/usr \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/.libs/iperf3 /usr/bin/iperf3
	cp -fP $(THISDIR)/$(SRC_NAME)/src/.libs/libiperf.so* $(ROMFSDIR)/lib
