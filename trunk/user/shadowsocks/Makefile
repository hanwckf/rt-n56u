SHADOWSOCKS_BIN=ssr-redir
THISDIR = $(shell pwd)

all:
	chmod -R +x scripts/
	@echo "shadowsocks done"

clean:
	@echo "shadowsocks done"

romfs:
	$(ROMFSINST) -d -S $(THISDIR)/bin/$(SHADOWSOCKS_BIN) /usr/bin/ss-redir
	$(ROMFSINST) -d $(THISDIR)/bin/ss-tunnel /usr/bin/ss-tunnel
	$(ROMFSINST) -d $(THISDIR)/bin/ss-rules /usr/bin/ss-rules
	$(ROMFSINST) -d $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -d $(THISDIR)/chnroute.txt /etc_ro/chnroute.txt
