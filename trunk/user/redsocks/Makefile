SRC_NAME=redsocks-release-0.5-11-g19b822e
SRC_URL=https://github.com/SuzukiHonoka/redsocks_for_mipsel/releases/download/release-0.5-11-g19b822e/redsocks

THISDIR = $(shell pwd)

all: download_test

download_test:
	( if [ ! -f redsocks ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O redsocks $(SRC_URL); \
	fi )

romfs:
	$(ROMFSINST) $(THISDIR)/redsocks /usr/bin/redsocks
	$(ROMFSINST) $(THISDIR)/redsocks.sh /usr/bin/redsocks.sh
	$(ROMFSINST) -p +x /usr/bin/redsocks.sh