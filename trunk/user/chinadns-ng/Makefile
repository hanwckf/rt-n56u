SRC_NAME = chinadns-ng
THISDIR = $(shell pwd)

all: 
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/chinadns-ng /usr/bin/chinadns-ng

