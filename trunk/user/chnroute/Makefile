url = http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest
THISDIR = $(shell pwd)

all :
	rm -f chnroute.txt.tmp
	( if [ ! -f chnroute.txt ];then \
		wget -t3 --timeout=5 -O- '$(url)' | awk -F\| '/CN\|ipv4/ { printf("%s/%d\n", $$4, 32-log($$5)/log(2)) }' > ./chnroute.txt.tmp ;\
		mv -f chnroute.txt.tmp chnroute.txt ;\
	fi )

romfs:
	chmod -R +x ./scripts
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/chnroute.txt /etc_ro/chnroute.txt

clean :
	rm -f chnroute.txt*
