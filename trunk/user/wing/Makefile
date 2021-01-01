THISDIR = $(shell pwd)

all:

clean:

romfs:
	$(ROMFSINST) $(THISDIR)/gfwlist.txt /etc_ro/gfwlist.txt
	$(ROMFSINST) -p +x $(THISDIR)/wing /usr/bin/wing

