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

