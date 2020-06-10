THISDIR = $(shell pwd)

all:

config_test:

clean:

romfs:
	$(ROMFSINST) $(THISDIR)/gfwlist.conf /etc_ro/gfwlist.conf
	$(ROMFSINST) -p +x $(THISDIR)/trojan /usr/bin/trojan
	$(ROMFSINST) -p +x $(THISDIR)/wing /usr/bin/wing
