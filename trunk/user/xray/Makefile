include $(ROOTDIR)/coustom
THISDIR = $(shell pwd)
#export GO111MODULE=on
#export GOPROXY=https://goproxy.bj.bcebos.com/
Xray_VERSION := 1.5.3
Xray_URL := https://codeload.github.com/XTLS/Xray-core/tar.gz/v$(Xray_VERSION)
Xray_dir = xray-core/Xray-core-$(Xray_VERSION)/main
ifeq ($(GITHUB_ACTION),n)
all:download_xray build_extract build_Xray

download_xray:
	( if [ ! -f $(THISDIR)/Xray-core-$(Xray_VERSION).tar.gz ]; then \
	curl --create-dirs -L $(Xray_URL) -o $(THISDIR)/Xray-core-$(Xray_VERSION).tar.gz ; \
	fi )

build_extract:
	mkdir -p $(THISDIR)/xray-core
	mkdir -p $(THISDIR)/bin
	( if [ ! -d $(THISDIR)/xray-core/Xray-core-$(Xray_VERSION) ]; then \
	rm -rf $(THISDIR)/xray-core/* ; \
	tar zxfv $(THISDIR)/Xray-core-$(Xray_VERSION).tar.gz -C $(THISDIR)/xray-core ; \
	fi )

build_Xray:
	( cd $(THISDIR)/$(Xray_dir); \
	if [ $(GOPROXY_ON) = "y" ]; then \
	go env -w GOPROXY=https://goproxy.cn,direct ; \
	fi ; \
	GOOS=linux GOARCH=mipsle go build -ldflags "-w -s" -o $(THISDIR)/bin/xray; \
	)
else
all:
endif
clean:
	rm -rf $(THISDIR)/xray-core
	rm -rf $(THISDIR)/bin

romfs:
ifeq ($(GITHUB_ACTION),n)	
	$(ROMFSINST) -p +x $(THISDIR)/bin/xray /usr/bin/v2ray
else
	$(ROMFSINST) -p +x $(THISDIR)/xray /usr/bin/v2ray
endif
