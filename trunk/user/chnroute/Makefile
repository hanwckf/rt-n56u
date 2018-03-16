url = http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest
THISDIR = $(shell pwd)

all :
	( if [ ! -f chnroute.txt ];then \
		wget -t3 --timeout=5 -O- '$(url)' | awk -F\| '/CN\|ipv4/ { printf("%s/%d\n", $$4, 32-log($$5)/log(2)) }' > ./chnroute.txt.tmp ;\
		mv -f chnroute.txt.tmp chnroute.txt ;\
	fi )
	tar jcf chnroute.bz2 chnroute.txt

romfs:
	chmod -R +x ./scripts
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/chnroute.bz2 /etc_ro/chnroute.bz2

clean :
	rm -f chnroute.*
