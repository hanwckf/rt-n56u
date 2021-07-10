THISDIR = $(shell pwd)

all:

config_test:

clean:

romfs:
	$(ROMFSINST) $(THISDIR)/gfwlist.txt /etc_ro/gfwlist.txt
	$(ROMFSINST) -p +x $(THISDIR)/trojan /usr/bin/trojan
	$(ROMFSINST) -p +x $(THISDIR)/wing /usr/bin/wing
