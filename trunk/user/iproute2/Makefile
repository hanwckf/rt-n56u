SRC_NAME=iproute2-3.4.0

THISDIR = $(shell pwd)

IPROUTE2_ENABLED=
ifeq ($(CONFIG_FIRMWARE_INCLUDE_QOS),y)
IPROUTE2_ENABLED=y
endif
ifdef CONFIG_IPV6
IPROUTE2_ENABLED=y
endif

all:
ifeq ($(IPROUTE2_ENABLED),y)
	$(MAKE) -C $(SRC_NAME)
else
	exit 0
endif

clean:
	$(MAKE) -C $(SRC_NAME) distclean CONFIG_IPV6=y CONFIG_XFRM=y

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_QOS),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/tc/tc /bin/tc
endif
ifdef CONFIG_IPV6
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ip/ip /bin/ip
endif
