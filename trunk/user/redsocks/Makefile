SRC_NAME := redsocks-19b822e
THISDIR := $(shell pwd)

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	( if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi )

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/redsocks /usr/bin/redsocks
#	$(ROMFSINST) -p +x $(THISDIR)/redsocks.sh /usr/bin/redsocks.sh
