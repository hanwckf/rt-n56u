url = http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest
THISDIR = $(shell pwd)

all :
	( if [ ! -f chnroute.bz2 ];then \
		wget -t3 -4 --timeout=5 -O chnroute.txt.tmp '$(url)' && \
		awk -F\| '/CN\|ipv4/ { printf("%s/%d\n", $$4, 32-log($$5)/log(2)) }' < chnroute.txt.tmp > chnroute.txt && \
		rm -f chnroute.txt.tmp && tar jcf chnroute.bz2 chnroute.txt ;\
	fi )

romfs:
	chmod -R +x ./scripts
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/chnroute.bz2 /etc_ro/chnroute.bz2

clean :
	rm -f chnroute.*
