THISDIR = $(shell pwd)
all:

clean:

romfs:
	$(ROMFSINST) /etc_ro/adbyby.tar.gz
	$(ROMFSINST) /etc_ro/adbyby_rules.sh
	$(ROMFSINST) /etc_ro/adbyby_adblack.sh
	$(ROMFSINST) /etc_ro/adbyby_adesc.sh
	$(ROMFSINST) /etc_ro/adbyby_adhost.sh
	$(ROMFSINST) /etc_ro/adbyby_blockip.sh
	$(ROMFSINST) -p +x $(THISDIR)/adbyby.sh /usr/bin/adbyby.sh