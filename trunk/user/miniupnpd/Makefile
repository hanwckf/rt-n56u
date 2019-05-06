SRC_NAME=miniupnpd-2.x

# reference to iptables package
IPT_VERSION:=iptables-1.4.16.3

# disable support IPv6 yet, because no profit w/o IGDv2
# e.g. Windows not supported IGDv2 and UPnP not worked
ENABLE_IPV6:=0

IPTABLESPATH=$(ROOTDIR)/user/iptables/$(IPT_VERSION)

all: config_test
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) IPTABLESPATH=$(IPTABLESPATH) ENABLE_IPV6=$(ENABLE_IPV6)

clean:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) clean
	rm -f config_done
	rm -f miniupnpd

romfs:
	cp $(SRC_NAME)/miniupnpd .
	$(ROMFSINST) /usr/bin/miniupnpd

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )
	
configure:
	cd $(SRC_NAME) && ./genconfig.sh
