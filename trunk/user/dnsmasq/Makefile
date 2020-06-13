SRC_NAME = dnsmasq-2.81
THISDIR = $(shell pwd)

COPTS := -DHAVE_BROKEN_RTC -DNO_INOTIFY -DNO_AUTH -DNO_LOOP
ifndef CONFIG_IPV6
COPTS += -DNO_IPV6
endif
ifndef CONFIG_USB_SUPPORT
COPTS += -DNO_TFTP
endif
ifneq ($(CONFIG_FIRMWARE_INCLUDE_IPSET),y)
COPTS += -DNO_IPSET
endif

all: extract_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) COPTS="$(COPTS)"

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz ; \
		patch -d $(SRC_NAME) -p1 -i ../patch/900-filter-aaaa.patch; \
		patch -d $(SRC_NAME) -p1 -i ../patch/910-mini-ttl.patch; \
	fi )

clean:
	rm -rf $(SRC_NAME)
	rm -f dnsmasq

romfs:
	cp $(SRC_NAME)/src/dnsmasq .
	$(STRIP) dnsmasq
	$(ROMFSINST) /usr/sbin/dnsmasq
	$(ROMFSINST) -p +x $(THISDIR)/dnsmasq.sh /usr/bin/dnsmasq.sh
