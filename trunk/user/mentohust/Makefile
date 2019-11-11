SRC_NAME = mentohust-1-master
#https://github.com/hanwckf/mentohust-1
THISDIR = $(shell pwd)

LDFLAGS += -L$(STAGEDIR) -liconv

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)


extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(SRC_NAME).zip; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	autoreconf -fi ; \
	am_cv_func_iconv=no \
	./configure \
		--prefix=/usr \
		--with-pcap=dylib \
		--disable-notify \
		--disable-encodepass \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/mentohust /usr/bin/bin_mentohust
	$(ROMFSINST) -p +x $(THISDIR)/scripts/mentohust.sh /usr/bin/mentohust.sh

