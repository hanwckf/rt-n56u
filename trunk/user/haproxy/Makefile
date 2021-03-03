# Makefile and scrpit made by Starx
BIN_NAME=haproxy
SRC_NAME=$(BIN_NAME).sh
BIN_URL=https://github.com/haproxy/haproxy
SRC_URL=https://raw.githubusercontent.com/SuzukiHonoka/Haproxy_Padavan/master/$(SRC_NAME)
BIN_PATH=/usr/bin

THISDIR = $(shell pwd)

all: bin_download src_download
	$(MAKE) -C $(BIN_NAME) ARCH=mips_24kc LDFLAGS=-static CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc TARGET=generic -j$(HOST_NCPU)
	$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-strip $(BIN_NAME)/$(BIN_NAME)

bin_download:
	( if [ ! -d $(BIN_NAME) ];then \
		git clone --depth=1 $(BIN_URL) --single-branch; \
	fi )
	

src_download:
	( if [ ! -f $(SRC_NAME) ]; then \
		wget $(SRC_URL); \
	fi )

clean:
	rm -rf $(THISDIR)/$(BIN_NAME) && rm $(THISDIR)/$(SRC_NAME)

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(BIN_NAME)/$(BIN_NAME) $(BIN_PATH)/$(BIN_NAME)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME) $(BIN_PATH)/$(SRC_NAME)
