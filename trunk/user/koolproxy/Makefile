THISDIR = $(shell pwd)
all:

clean:

romfs:
	$(ROMFSINST) /etc_ro/koolproxy.tgz
	$(ROMFSINST) /etc_ro/ad_config_script.sh
	$(ROMFSINST) /etc_ro/koolproxy_rules_list.sh
	$(ROMFSINST) /etc_ro/koolproxy_rules_script.sh
	$(ROMFSINST) -p +x $(THISDIR)/koolproxy.sh /usr/bin/koolproxy.sh