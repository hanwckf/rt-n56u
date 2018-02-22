dns_server=114.114.114.114
acc_china_file=accelerated-domains.china.conf
acc_apple_file=apple.china.conf
url=https://coding.net/u/felixonmars/p/dnsmasq-china-list/git/raw/master
dir=dnsmasq-china-conf

THISDIR = $(shell pwd)

all:
	( if [ ! -f $(dir)/$(acc_china_file) ];then \
		wget -q -t5 --timeout=20 --no-check-certificate -P $(dir) $(url)/$(acc_china_file); \
	fi )
	( if [ ! -f $(dir)/$(acc_apple_file) ];then \
		wget -q -t5 --timeout=20 --no-check-certificate -P $(dir) $(url)/$(acc_apple_file); \
	fi )
	cat $(dir)/$(acc_apple_file) >> $(dir)/$(acc_china_file)
	sed -i "s|^\(server.*\)/[^/]*$$|\1/$(dns_server)|" $(dir)/$(acc_china_file)

clean:
	rm -rf $(dir)

romfs:
	chmod -R +x ./scripts
	$(ROMFSINST) $(THISDIR)/scripts /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/$(dir)/$(acc_china_file) /etc_ro/$(dir)/$(acc_china_file)
