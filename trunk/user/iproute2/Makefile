SRC_NAME=iproute2-3.1.0

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	rm -f tc
	rm -f ip
	rm -f m_xt.so
	$(MAKE) -C $(SRC_NAME) distclean

romfs:
	cp $(SRC_NAME)/tc/tc .
	$(ROMFSINST) /bin/tc
ifeq ($(KERNEL3X),y)
	cp $(SRC_NAME)/tc/m_xt.so .
	$(ROMFSINST) /usr/lib/xtables/m_xt.so
endif
ifdef CONFIG_IPV6
	cp $(SRC_NAME)/ip/ip .
	$(ROMFSINST) /bin/ip
endif
