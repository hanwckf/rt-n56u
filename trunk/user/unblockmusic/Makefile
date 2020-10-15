THISDIR = $(shell pwd)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
UM_VERSION := 0.2.4
UM_URL := https://github.com/cnsilvan/UnblockNeteaseMusic/archive/$(UM_VERSION).tar.gz

all:download_um build_extract build

build_extract:
	mkdir -p $(THISDIR)/UnblockNeteaseMusicGo
	mkdir -p $(THISDIR)/bin
	( if [ ! -d $(THISDIR)/UnblockNeteaseMusicGo/UnblockNeteaseMusic-$(UM_VERSION) ]; then \
	tar zxfv $(THISDIR)/UnblockNeteaseMusic-$(UM_VERSION).tar.gz -C $(THISDIR)/UnblockNeteaseMusicGo ; \
	fi )

build:
	( cd $(THISDIR)/UnblockNeteaseMusicGo/UnblockNeteaseMusic-$(UM_VERSION); \
	GOOS=linux GOARCH=mipsle go build -ldflags "-w -s" -o $(THISDIR)/bin/UnblockNeteaseMusic; \
	) 

download_um:
	( if [ ! -f $(THISDIR)/UnblockNeteaseMusic-$(UM_VERSION).tar.gz ]; then \
	curl --create-dirs -L $(UM_URL) -o $(THISDIR)/UnblockNeteaseMusic-$(UM_VERSION).tar.gz ; \
	fi )

clean:
	rm -rf $(THISDIR)/UnblockNeteaseMusicGo
	rm -rf $(THISDIR)/bin

romfs:
	$(ROMFSINST) $(THISDIR)/file/ /etc_ro/UnblockNeteaseMusicGo/
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -p +x $(THISDIR)/bin/UnblockNeteaseMusic /usr/bin/UnblockNeteaseMusic
