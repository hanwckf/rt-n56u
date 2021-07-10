SRC_NAME = dns2tcp
THISDIR = $(shell pwd)

all: 
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) CC="$(CC)"

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/dns2tcp /usr/bin/dns2tcp

