SRC_NAME = mentohust-master
ARCHIVE_NAME := mentohust-6a6da8d
#https://github.com/hanwckf/mentohust
THISDIR = $(shell pwd)

LDFLAGS += -L$(STAGEDIR) -liconv

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)


extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(ARCHIVE_NAME).zip; \
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
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/mentohust /usr/bin/mentohust
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/mentohust.conf /etc_ro/mentohust-sample.conf
