SRC_NAME=iproute2-3.1.0

THISDIR = $(shell pwd)

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	$(MAKE) -C $(SRC_NAME) distclean

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/tc/tc /bin/tc
ifdef CONFIG_IPV6
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ip/ip /bin/ip
endif
