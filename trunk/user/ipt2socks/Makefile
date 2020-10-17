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
