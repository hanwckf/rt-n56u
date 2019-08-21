SRC_NAME = coremark-2e9d6165f362f7071d29b7f88c6d32655e5d4a3c
THISDIR = $(shell pwd)

all: extract_test
	$(MAKE) -C $(SRC_NAME) XCFLAGS="-DMULTITHREAD=8 -DUSE_PTHREAD" PORT_DIR=linux LFLAGS_END="-lpthread -lrt" compile

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(SRC_NAME).zip ; \
		sed -i 's#CC = gcc##' $(SRC_NAME)/linux/core_portme.mak ; \
	fi )

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) PORT_DIR=linux clean ; \
	fi ; 

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/coremark.exe /usr/bin/coremark
