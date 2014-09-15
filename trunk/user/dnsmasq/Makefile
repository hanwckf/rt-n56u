SRC_NAME=dnsmasq-2.7x

all:
	$(MAKE) -C $(SRC_NAME) COPTS="$(if $(CONFIG_IPV6),,-DNO_IPV6) $(if $(CONFIG_USB_SUPPORT),,-DNO_TFTP)"

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -f dnsmasq

romfs:
	cp $(SRC_NAME)/src/dnsmasq .
	$(STRIP) dnsmasq
	$(ROMFSINST) /usr/sbin/dnsmasq

