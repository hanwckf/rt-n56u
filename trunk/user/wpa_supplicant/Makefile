SRC_NAME=wpa_supplicant

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	$(MAKE) -C $(SRC_NAME) clean

romfs:
	$(STRIP) $(SRC_NAME)/wpa_supplicant
	$(STRIP) $(SRC_NAME)/wpa_cli
	cd $(SRC_NAME) && $(ROMFSINST) /usr/sbin/wpa_cli
	cd $(SRC_NAME) && $(ROMFSINST) /usr/sbin/wpa_supplicant
