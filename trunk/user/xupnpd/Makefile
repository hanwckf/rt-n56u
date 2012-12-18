SRC_NAME=src

all:
	make -C $(SRC_NAME) embedded

clean:
	make -C $(SRC_NAME) clean

romfs:
	mkdir -p $(ROMFSDIR)/etc_ro/xupnpd
	mkdir -p $(ROMFSDIR)/usr/share/xupnpd
	$(ROMFSINST) $(SRC_NAME)/xupnpd /usr/bin/xupnpd
	cp -f $(SRC_NAME)/*.lua $(ROMFSDIR)/etc_ro/xupnpd
	cp -rf $(SRC_NAME)/ui $(ROMFSDIR)/usr/share/xupnpd
	cp -rf $(SRC_NAME)/www $(ROMFSDIR)/usr/share/xupnpd
	cp -rf $(SRC_NAME)/profiles $(ROMFSDIR)/etc_ro/xupnpd
	cp -rf $(SRC_NAME)/plugins $(ROMFSDIR)/etc_ro/xupnpd
