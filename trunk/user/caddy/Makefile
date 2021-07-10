THISDIR := $(shell pwd)

all: 

download_test:

extra_test:

clean:

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/caddy.sh /usr/bin/caddy.sh
	$(ROMFSINST) /etc_ro/caddy_script.sh
ifeq ($(CONFIG_FIRMWARE_INCLUDE_CADDYBIN),y)
	$(ROMFSINST) -p +x $(THISDIR)/caddy_filebrowser /usr/bin/caddy_filebrowser
endif
