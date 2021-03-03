BIN_NAME="ipt2socks"
SRC_NAME="$(BIN_NAME).sh"
BIN_URL="https://github.com/zfl9/ipt2socks"
SRC_URL="https://raw.githubusercontent.com/SuzukiHonoka/Ipt2socks_Padavan/master/$(SRC_NAME)"
BIN_PATH="/usr/bin"

THISDIR = $(shell pwd)

all: bin_download src_download
	$(MAKE) -C $(BIN_NAME) ARCH=mips_24kc CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc -j$(HOST_NCPU)
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
	$(ROMFSINST) -p +x $(THISDIR)/$(BIN_NAME)/$(BIN_NAME) $(BIN_PATH)/$(BIN_NAME)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME) $(BIN_PATH)/$(SRC_NAME)
