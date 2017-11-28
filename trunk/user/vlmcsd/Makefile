THISDIR = $(shell pwd)
SRC_NAME = vlmcsd
git_commit_id = 8f85825cf108e1ac52c164bc20f9c8f537e4810c

all: download_test
	$(MAKE) -C $(SRC_NAME) -B CC=$(ROOTDIR)/../toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-gcc
	
download_test: 
	( if [ ! -d $(THISDIR)/$(SRC_NAME)/ ]; then \
		git clone https://github.com/hanwckf/vlmcsd.git $(SRC_NAME); \
		cd $(SRC_NAME); \
		git reset --hard $(git_commit_id);\
	fi )
romfs:
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/bin/vlmcsd /usr/bin/vlmcsd
	$(ROMFSINST) $(THISDIR)/vlmcsd.sh /usr/bin/vlmcsd.sh
clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
