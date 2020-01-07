SRC_NAME = smartdns
THISDIR = $(shell pwd)

all: config_test
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMARTDNSBIN),y)
	$(MAKE) -C $(SRC_NAME) CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc
endif

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		touch config_done; \
	fi )

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMARTDNSBIN),y)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/smartdns /usr/bin/smartdns
endif
	$(ROMFSINST) -p +x $(THISDIR)/smartdns.sh /usr/bin/smartdns.sh
	$(ROMFSINST) /etc_ro/smartdns_address.conf
	$(ROMFSINST) /etc_ro/smartdns_blacklist-ip.conf
	$(ROMFSINST) /etc_ro/smartdns_custom.conf
	$(ROMFSINST) /etc_ro/smartdns_whitelist-ip.conf
