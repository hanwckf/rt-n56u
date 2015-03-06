SRC_NAME=mtd-utils-1.5.x

THISDIR = $(shell pwd)

all:
	$(MAKE) -C $(SRC_NAME) WITHOUT_XATTR=1 BUILDDIR=$(THISDIR)/build

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -rf $(THISDIR)/build

romfs:
	$(ROMFSINST) $(THISDIR)/build/ubi-utils/ubiattach /sbin/ubiattach
	$(ROMFSINST) $(THISDIR)/build/ubi-utils/ubidetach /sbin/ubidetach
	$(ROMFSINST) $(THISDIR)/build/ubi-utils/ubiformat /sbin/ubiformat
	$(ROMFSINST) $(THISDIR)/build/ubi-utils/ubimkvol /sbin/ubimkvol
	$(ROMFSINST) $(THISDIR)/build/ubi-utils/ubinfo /sbin/ubinfo
