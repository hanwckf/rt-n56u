THISDIR := $(shell pwd)
FRP_VER := 0.29.0
FRP_URL_BASE := https://github.com/fatedier/frp/releases/download

FRP_NAME := frp_$(FRP_VER)_linux_mipsle
FRP_URL := $(FRP_URL_BASE)/v$(FRP_VER)/$(FRP_NAME).tar.gz

all: download_test extra_test
	@echo "frp build done!"

download_test:
	( if [ ! -f $(FRP_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(FRP_NAME).tar.gz $(FRP_URL); \
	fi )

extra_test:
	( if [ ! -d $(FRP_NAME) ]; then \
		tar xf $(FRP_NAME).tar.gz; \
	fi )

clean:
	rm -rf $(FRP_NAME).tar.gz $(FRP_NAME)

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FRPC),y)
	$(ROMFSINST) -p +x $(THISDIR)/$(FRP_NAME)/frpc /usr/bin/frpc
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FRPS),y)
	$(ROMFSINST) -p +x $(THISDIR)/$(FRP_NAME)/frps /usr/bin/frps
endif
