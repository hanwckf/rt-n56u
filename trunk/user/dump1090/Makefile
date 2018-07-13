SRC_NAME = dump1090-master
ARCHIVE_NAME = dump1090-master-0041922
#SRC_URL = https://github.com/hanwckf/dump1090/archive/master.zip

THISDIR = $(shell pwd)

all: extract_test
	$(MAKE) -C $(SRC_NAME)

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(ARCHIVE_NAME).zip; \
	fi )

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/dump1090 /usr/bin/dump1090
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/gmap.html /usr/bin/gmap.html
	$(ROMFSINST) -d $(THISDIR)/$(SRC_NAME)/tools/debug.html /www/dump1090_debug.html
	ln -sf /tmp/frames.js $(INSTALLDIR)/www/frames.js
