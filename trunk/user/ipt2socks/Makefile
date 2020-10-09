<<<<<<< HEAD
THISDIR = $(shell pwd)
SRC_NAME = ipt2socks
SRC_VERSION := 1.1.3
SRC_DIR := $(SRC_NAME)-$(SRC_VERSION)
SRC_URL := https://github.com/zfl9/ipt2socks/archive/v$(SRC_VERSION).tar.gz

all: download_src extract_src
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_DIR) CC="$(CC)"

download_src:
	( if [ ! -f $(THISDIR)/$(SRC_DIR).tar.gz ]; then \
	curl --create-dirs -L $(SRC_URL) -o $(THISDIR)/$(SRC_DIR).tar.gz ; \
	fi )	

extract_src:
	( if [ ! -d $(SRC_DIR) ]; then \
		tar zxf $(SRC_DIR).tar.gz ; \
	fi )

clean:
	rm -rf $(SRC_DIR)
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_DIR)/ipt2socks /usr/bin/ipt2socks
=======
SRC_NAME = ipt2socks-1.1.2
THISDIR = $(shell pwd)

all: extract_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) CC="$(CC)"
	
extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

clean:
	rm -rf $(SRC_NAME)
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/ipt2socks /usr/bin/ipt2socks
>>>>>>> a321e6940bb0cb44619e21b8b3df6e91f892751a

