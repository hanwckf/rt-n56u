THISDIR := $(shell pwd)

all: 

download_test:

extra_test:

clean:

romfs:
	$(ROMFSINST) $(THISDIR)/UnblockNeteaseMusicGo/ /etc_ro/UnblockNeteaseMusicGo/
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
ifeq ($(CONFIG_FIRMWARE_INCLUDE_WYYBIN),y)
	$(ROMFSINST) -p +x $(THISDIR)/UnblockNeteaseMusic /usr/bin/UnblockNeteaseMusic
endif
