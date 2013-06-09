SRC_NAME=miniupnpd-1.8.x

IPT_VERSION=iptables-1.4.16.3

all:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) $(IPT_PATH) IPTABLESPATH=$(ROOTDIR)/user/iptables/$(IPT_VERSION)

clean:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) clean
	rm -f miniupnpd

romfs:
	cp $(SRC_NAME)/miniupnpd .
	$(ROMFSINST) /usr/bin/miniupnpd

