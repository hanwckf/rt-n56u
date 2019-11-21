SRC_NAME=libusb-1.0.23
SRC_URL=https://github.com/libusb/libusb/releases/download/v1.0.23/libusb-1.0.23.tar.bz2

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.bz2 ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.bz2 $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar jxf $(SRC_NAME).tar.bz2; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	ac_cv_lib_rt_clock_gettime=no \
	./configure \
		--prefix=$(STAGEDIR) \
		--disable-static \
		--disable-log \
		--disable-udev \
		--disable-examples-build \
		--disable-tests-build \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:
	cp -fP $(DESTDIR)/lib/libusb-1.0.so* $(ROMFSDIR)/lib

