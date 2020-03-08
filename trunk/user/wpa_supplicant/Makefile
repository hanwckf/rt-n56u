SRC_NAME=wpa_supplicant


all:
	cp -f $(SRC_NAME)/.config.def $(SRC_NAME)/.config
ifeq ($(CONFIG_FIRMWARE_INCLUDE_EAP_PEAP),y)
	echo CONFIG_EAP_TTLS=y >> $(SRC_NAME)/.config
	echo CONFIG_EAP_PEAP=y >> $(SRC_NAME)/.config
	echo CONFIG_EAP_MSCHAPV2=y >> $(SRC_NAME)/.config
else ifeq (,$(wildcard $(STAGEDIR)/lib/pkgconfig/openssl.pc))
	echo CONFIG_TLS=internal >> $(SRC_NAME)/.config
	echo CONFIG_CRYPTO=internal >> $(SRC_NAME)/.config
	echo CONFIG_INTERNAL_LIBTOMMATH=y >> $(SRC_NAME)/.config
	echo CONFIG_INTERNAL_RC4=y >> $(SRC_NAME)/.config
endif
	$(MAKE) -C $(SRC_NAME)

clean:
	$(MAKE) -C $(SRC_NAME) clean

romfs:
	$(ROMFSINST) $(SRC_NAME)/wpa_cli /usr/sbin/wpa_cli
	$(ROMFSINST) $(SRC_NAME)/wpa_supplicant /usr/sbin/wpa_supplicant
