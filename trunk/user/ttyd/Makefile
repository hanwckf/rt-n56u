SRC_NAME=ttyd-1.6.2
SRC_URL=https://github.com/tsl0922/ttyd/archive/1.6.2.tar.gz

THISDIR = $(shell pwd)

CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build

download_test:
	( if [ ! -f $(THISDIR)/$(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake -DCMAKE_BUILD_TYPE=RELEASE .. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/build/ttyd /usr/bin/ttyd
	$(ROMFSINST) $(THISDIR)/ttyd.sh /usr/bin/ttyd.sh
