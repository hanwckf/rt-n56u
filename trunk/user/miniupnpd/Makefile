SRC_NAME=miniupnpd-1.8.x

IPT_VERSION=iptables-1.4.16.3

# disable support IPv6 yet, because no profit w/o IGDv2
# e.g. Windows not supported IGDv2 and UPnP not worked
ENABLE_IPV6=0

all:
ifeq ($(CONFIG_IPV6),y)
	cd $(SRC_NAME) && ./genconfig.sh --pcp
else
	cd $(SRC_NAME) && ./genconfig.sh
endif
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) IPTABLESPATH=$(ROOTDIR)/user/iptables/$(IPT_VERSION) ENABLE_IPV6=$(ENABLE_IPV6)

clean:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) clean
	rm -f miniupnpd

romfs:
	cp $(SRC_NAME)/miniupnpd .
	$(ROMFSINST) /usr/bin/miniupnpd

