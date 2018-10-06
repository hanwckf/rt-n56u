dns_server=114.114.114.114
acc_china_file=accelerated-domains.china.conf
acc_apple_file=apple.china.conf
archive_file=dnsmasq-china-conf.bz2
url=https://raw.githubusercontent.com/felixonmars/dnsmasq-china-list/master
#url=https://coding.net/u/felixonmars/p/dnsmasq-china-list/git/raw/master
dir=dnsmasq-china-conf

THISDIR = $(shell pwd)

all:
	mkdir -p $(dir)
	( if [ ! -f $(dir)/.finish ]; then \
		rm -f $(dir)/* && \
		wget -q -t5 --timeout=20 --no-check-certificate -P $(dir) $(url)/$(acc_china_file) && \
		wget -q -t5 --timeout=20 --no-check-certificate -P $(dir) $(url)/$(acc_apple_file) && \
		cat $(dir)/$(acc_apple_file) >> $(dir)/$(acc_china_file) && \
		rm -f $(dir)/$(acc_apple_file) && \
		touch $(dir)/.finish ; \
	fi )
	sed -i "s|^\(server.*\)/[^/]*$$|\1/$(dns_server)|" $(dir)/$(acc_china_file)
	cd $(dir) && tar jcf $(archive_file) $(acc_china_file)

clean:
	rm -rf $(dir)

romfs:
	chmod -R +x ./scripts
	$(ROMFSINST) $(THISDIR)/scripts /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/$(dir)/$(archive_file) /etc_ro/$(dir)/$(archive_file)
