SRC_NAME=libusb-1.0.8

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	export ac_cv_lib_rt_clock_gettime=no ; \
	./configure \
		--prefix=/ \
		--disable-static \
		--disable-timerfd \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	cp -fP $(DESTDIR)/lib/libusb-1*.so* $(ROMFSDIR)/lib

