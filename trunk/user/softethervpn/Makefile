THISDIR = $(shell pwd)
BIN_DIR = softether-v4.27-9666-beta

all:
	@echo "softethervpn done"

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CMD),y)
	$(ROMFSINST) -S -p +x $(THISDIR)/$(BIN_DIR)/vpncmd /etc_ro/vpncmd
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CLIENT),y)
	$(ROMFSINST) -S -p +x $(THISDIR)/$(BIN_DIR)/vpnclient /etc_ro/vpnclient
endif
	$(ROMFSINST) -S $(THISDIR)/$(BIN_DIR)/hamcore.se2 /etc_ro/hamcore.se2

clean:
	@echo "softethervpn done"
