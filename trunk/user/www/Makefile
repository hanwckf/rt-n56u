ROMFS_DIR = $(ROOTDIR)/romfs

LABEL_COPYRIGHT_TEXT="© 2011-2014 Padavan \& N56U project community"

BOARD_PATTERN=s/ZVMODELVZ/$(CONFIG_FIRMWARE_PRODUCT_ID)/g
COPYR_PATTERN=s/ZVCOPYRVZ/$(LABEL_COPYRIGHT_TEXT)/g

WEBUI_NAME=n56u_ribbon_fixed

all:

romfs:
	echo "Make www romfs"
	rm -Rf $(ROMFS_DIR)/www
	cp -R $(WEBUI_NAME) $(ROMFS_DIR)/www
	rm -f $(ROMFS_DIR)/www/httpd_check.htm
	echo "ASUSTeK" > $(ROMFS_DIR)/www/httpd_check.htm
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings.CFG && cd ../..
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings_$(CONFIG_FIRMWARE_PRODUCT_ID).CFG && cd ../..
	cd $(ROMFS_DIR)/www && ln -sf /opt/share/www/custom custom
	cp dict/EN.header $(ROMFS_DIR)/www
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_BR),y)
	echo "LANG_BR=Brazil" >> $(ROMFS_DIR)/www/EN.header
	cp dict/BR.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/BR.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/BR.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_CZ),y)
	echo "LANG_CZ=Česky" >> $(ROMFS_DIR)/www/EN.header
	cp dict/CZ.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/CZ.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/CZ.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_DA),y)
	echo "LANG_DA=Dansk" >> $(ROMFS_DIR)/www/EN.header
	cp dict/DA.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/DA.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/DA.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_DE),y)
	echo "LANG_DE=Deutsch" >> $(ROMFS_DIR)/www/EN.header
	cp dict/DE.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/DE.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/DE.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_ES),y)
	echo "LANG_ES=Español" >> $(ROMFS_DIR)/www/EN.header
	cp dict/ES.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/ES.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/ES.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_FI),y)
	echo "LANG_FI=Finsk" >> $(ROMFS_DIR)/www/EN.header
	cp dict/FI.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/FI.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/FI.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_FR),y)
	echo "LANG_FR=Français" >> $(ROMFS_DIR)/www/EN.header
	cp dict/FR.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/FR.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/FR.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_NO),y)
	echo "LANG_NO=Norsk" >> $(ROMFS_DIR)/www/EN.header
	cp dict/NO.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/NO.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/NO.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_PL),y)
	echo "LANG_PL=Polski" >> $(ROMFS_DIR)/www/EN.header
	cp dict/PL.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/PL.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/PL.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_RU),y)
	echo "LANG_RU=Pусский" >> $(ROMFS_DIR)/www/EN.header
	cp dict/RU.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/RU.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/RU.dict
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_LANG_SV),y)
	echo "LANG_SV=Svensk" >> $(ROMFS_DIR)/www/EN.header
	cp dict/SV.dict $(ROMFS_DIR)/www
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/SV.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/SV.dict
endif
	cat $(ROMFS_DIR)/www/EN.header dict/EN.footer > $(ROMFS_DIR)/www/EN.dict
	sed $(BOARD_PATTERN) -i $(ROMFS_DIR)/www/EN.dict
	sed $(COPYR_PATTERN) -i $(ROMFS_DIR)/www/EN.dict
	rm -f $(ROMFS_DIR)/www/EN.header
	rm -f $(ROMFS_DIR)/www/bootstrap/js/bootstrap.js
	rm -f $(ROMFS_DIR)/www/bootstrap/js/engage.itoggle.js
	rm -f $(ROMFS_DIR)/www/bootstrap/js/jquery.backgroundpos.min.js
	rm -f $(ROMFS_DIR)/www/bootstrap/js/jquery.easing.1.3.js
	rm -f $(ROMFS_DIR)/www/bootstrap/css/bootstrap.css

clean:
	echo "Clean www romfs"
