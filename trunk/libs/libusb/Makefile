SRC_NAME=libusb-0.1.12

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
	./configure \
		--prefix=/ \
		--disable-static \
		--disable-build-docs \
		--enable-debug=no \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	cp -fP $(DESTDIR)/lib/libusb-0.1.so* $(ROMFSDIR)/lib
	cp -fP $(DESTDIR)/lib/libusb.so $(ROMFSDIR)/lib

