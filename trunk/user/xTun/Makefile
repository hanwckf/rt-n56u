SRC_NAME = xTun-0.6.3
SRC_URL = https://github.com/lparam/xTun/archive/v0.6.3.tar.gz
LIBUV_SRC_NAME = libuv-1.22.0
LIBUV_SRC_URL = https://github.com/libuv/libuv/archive/v1.22.0.tar.gz
LIBSODIUM__SRC_NAME = libsodium-1.0.16
LIBSODIUM_SRC_URL = https://github.com/jedisct1/libsodium/archive/1.0.16.tar.gz
THISDIR = $(shell pwd)

all: download_test extract_test
	$(MAKE) -C $(SRC_NAME) HOST=$(HOST_TARGET)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )
	( if [ ! -f $(LIBUV_SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(LIBUV_SRC_NAME).tar.gz $(LIBUV_SRC_URL); \
	fi )
	( if [ ! -f $(LIBSODIUM__SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(LIBSODIUM__SRC_NAME).tar.gz $(LIBSODIUM_SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.gz; \
	fi )
	( if [ ! -d $(SRC_NAME)/3rd/$(LIBUV_SRC_NAME) ]; then \
		tar -xf $(LIBUV_SRC_NAME).tar.gz -C $(SRC_NAME)/3rd/; \
		rm -rf $(SRC_NAME)/3rd/libuv; ln -s $(THISDIR)/$(SRC_NAME)/3rd/$(LIBUV_SRC_NAME) $(SRC_NAME)/3rd/libuv; \
	fi )
	( if [ ! -d $(SRC_NAME)/3rd/$(LIBSODIUM__SRC_NAME) ]; then \
		tar -xf $(LIBSODIUM__SRC_NAME).tar.gz -C $(SRC_NAME)/3rd/; \
		rm -rf $(SRC_NAME)/3rd/libsodium; ln -s $(THISDIR)/$(SRC_NAME)/3rd/$(LIBSODIUM__SRC_NAME) $(SRC_NAME)/3rd/libsodium; \
	fi )

clean:
	$(MAKE) -C $(SRC_NAME) clean

romfs:
	$(STRIP) $(THISDIR)/$(SRC_NAME)/xTun
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/xTun /usr/bin/xTun
	$(ROMFSINST) -p +x $(THISDIR)/xTun.sh /usr/bin/xTun.sh