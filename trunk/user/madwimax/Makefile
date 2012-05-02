SRC_NAME=madwimax-0.1.1

PWD_DIR=$(shell pwd)

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
		libusb1_CFLAGS=-I$(ROOTDIR)/stage/include/libusb-1.0 \
		libusb1_LIBS=-lusb-1.0 \
		--prefix=/usr \
		--with-script=wl500g \
		--without-man-pages \
		--with-udev-dir=$(PWD_DIR)/$(SRC_NAME)/build \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -f madwimax
	rm -f event.sh
	rm -f config_done

romfs:
	cp -f $(SRC_NAME)/src/madwimax .
	cp -f $(SRC_NAME)/scripts/events/event.sh .
	$(STRIP) madwimax
	$(ROMFSINST) /usr/sbin/madwimax
	$(ROMFSINST) -p +x /usr/sbin/event.sh

