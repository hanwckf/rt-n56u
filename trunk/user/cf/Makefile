BIN_NAME=cf
SRC_NAME=$(BIN_NAME).sh
BIN_URL=https://github.com/SuzukiHonoka/Cloudflare_DDNS/releases/latest/download/cf
SRC_URL=https://raw.githubusercontent.com/SuzukiHonoka/Cloudflare-DDNS_Padavan/master/$(SRC_NAME)
BIN_PATH=/usr/bin

THISDIR = $(shell pwd)

all: bin_download src_download
	chmod +x $(THISDIR)/$(BIN_NAME)
	$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-strip $(THISDIR)/$(BIN_NAME)
	upx --best --lzma $(THISDIR)/$(BIN_NAME)

bin_download:
	( if [ ! -f $(BIN_NAME) ];then \
		wget $(BIN_URL); \
	fi )
	

src_download:
	( if [ ! -f $(SRC_NAME) ]; then \
		wget $(SRC_URL); \
	fi )

clean:
	rm -rf $(THISDIR)/$(BIN_NAME) && rm $(THISDIR)/$(SRC_NAME)

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(BIN_NAME) $(BIN_PATH)/$(BIN_NAME)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME) $(BIN_PATH)/$(SRC_NAME)
