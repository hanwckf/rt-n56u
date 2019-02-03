SRC_NAME=mtd-utils-2.0.2

THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xf $(SRC_NAME).tar.bz2; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix= \
		--enable-static \
		--disable-shared \
		--disable-tests \
		--without-jffs \
		--without-xattr \
		--without-ubifs \
		--without-lzo \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ubiattach /sbin/ubiattach
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ubidetach /sbin/ubidetach
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ubiformat /sbin/ubiformat
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ubimkvol /sbin/ubimkvol
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ubinfo /sbin/ubinfo
