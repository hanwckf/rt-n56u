all:

romfs:
	mkdir -p $(INSTALLDIR)/etc_ro/opt
	$(ROMFSINST) /etc_ro/ipkg.conf
	$(ROMFSINST) /etc_ro/opt/K95aria2
	$(ROMFSINST) /etc_ro/opt/K95transmission
	$(ROMFSINST) -p +x /usr/bin/ipkg.sh
	$(ROMFSINST) -p +x /usr/bin/opkg.sh
	$(ROMFSINST) -p +x /usr/bin/opt-mount.sh
	$(ROMFSINST) -p +x /usr/bin/opt-umount.sh
	$(ROMFSINST) -p +x /usr/bin/opt-ipkg-upd.sh
	$(ROMFSINST) -p +x /usr/bin/opt-opkg-upd.sh
	$(ROMFSINST) -p +x /usr/bin/opt-mkswap

clean:

