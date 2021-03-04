BIN_NAME=smartdns
SRC_NAME=$(BIN_NAME).sh
BIN_URL=https://github.com/pymumu/smartdns
SRC_URL=https://raw.githubusercontent.com/SuzukiHonoka/Smartdns_Padavan/master/$(SRC_NAME)
BIN_PATH=/usr/bin

THISDIR = $(shell pwd)

all: bin_download src_download
	ln -s $(STAGEDIR)/lib/*.so $(CONFIG_CROSS_COMPILER_ROOT)/lib
	$(MAKE) -C $(BIN_NAME) ARCH=mips_24kc CFLAGS="-I$(STAGEDIR)/include" LDFLAGS="-L$(STAGEDIR)/lib -Wl,-rpath-link=$(STAGEDIR)/lib" -j$(HOST_NCPU)
bin_download:
	( if [ ! -f $(BIN_NAME) ]; then \
		git clone --depth=1 --single-branch $(BIN_URL); \
	fi )

src_download:
	( if [ ! -f $(SRC_NAME) ]; then \
		wget $(SRC_URL); \
	fi )

clean:
	rm -r $(THISDIR)/$(BIN_NAME) && rm $(THISDIR)/$(SRC_NAME)

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(BIN_NAME)/src/$(BIN_NAME) $(BIN_PATH)/$(BIN_NAME)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME) $(BIN_PATH)/$(SRC_NAME)
