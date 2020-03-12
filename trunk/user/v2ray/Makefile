THISDIR = $(shell pwd)
SRC_NAME = v2ray-linux-mipsle
SRC_URL = https://github.com/v2ray/v2ray-core/releases/download/v4.22.1/v2ray-linux-mipsle.zip

all: download_test extract_test upx

	
download_test: 
	( if [ ! -f $(THISDIR)/$(SRC_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).zip $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/v2ray ]; then \
		unzip $(SRC_NAME).zip -d v2ray ; \
	fi )

upx:
	cd v2ray ; \
	command -v upx >/dev/null 2>&1 || { echo >&2 "I require upx but it's not installed.  installing."; sudo install upx; } ; \
	upx -k --best --lzma ./v2ray ; \
	upx -k --best --lzma ./v2ctl ; \
	mkdir v2c ; \
	cp config.json v2c ; \

romfs:

	$(ROMFSINST) -p +x $(THISDIR)/v2ray/v2ray /usr/bin/v2ray
	$(ROMFSINST) -p +x $(THISDIR)/v2ray/v2ctl /usr/bin/v2ctl
	$(ROMFSINST) -p +x $(THISDIR)/v2ray/v2c /etc/v2c

clean:
	if [ -d v2ray ] ; then \
		rm -rf v2ray ; \
	fi ; \
