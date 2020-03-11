THISDIR = $(shell pwd)
SRC_NAME = N2N_MIPSEL_CROSS_COMPILE
SRC_URL = https://github.com/SuzukiHonoka/N2N_MIPSEL_CROSS_COMPILE
TOOLCHAIN_URL = https://github.com/SuzukiHonoka/Padavan_Toolchain3.4.x_EXTRA

all: download_test toolchain_update
	$(MAKE) -C $(SRC_NAME) CC=$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc
	
download_test: 
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		git clone --depth=1 $(SRC_URL) ; \
	fi )

toolchain_update:
	( if [ ! -d $(CONFIG_CROSS_COMPILER_ROOT)/include/openssl ]; then \
		git clone $(TOOLCHAIN_URL) ; \
		cd Padavan_Toolchain3.4.x_EXTRA ; \
		cp -r include $(CONFIG_CROSS_COMPILER_ROOT) ; \
		cp -r lib $(CONFIG_CROSS_COMPILER_ROOT) ; \
		cp -r mipsel-linux-uclibc/lib $(CONFIG_CROSS_COMPILER_ROOT)/mipsel-linux-uclibc ; \
	fi )

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/edge /usr/bin/edge
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/supernode /usr/bin/supernode

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
