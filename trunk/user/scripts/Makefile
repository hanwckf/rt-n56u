
all:

clean:

romfs:
	$(ROMFSINST) /sbin/dev_init.sh
	$(ROMFSINST) /sbin/reset_ss.sh
	$(ROMFSINST) /sbin/mtd_storage.sh
ifdef CONFIG_USB_SUPPORT
	$(ROMFSINST) /sbin/automount.sh
	$(ROMFSINST) /sbin/hddtune.sh
	$(ROMFSINST) /sbin/spindown.sh
endif
ifdef CONFIG_RTL8367
	$(ROMFSINST) /sbin/rtl8367mib
endif
	./makedevlinks $(ROMFSDIR)
	$(ROMFSINST) /etc_ro/ld.so.conf
	$(ROMFSINST) /etc_ro/protocols
	$(ROMFSINST) /etc_ro/services
	$(ROMFSINST) /etc_ro/profile
	$(ROMFSINST) /etc_ro/shells
	mkdir -p $(INSTALLDIR)/usr/share/terminfo
	tar -xzf ./terminfo.tar.gz -C $(INSTALLDIR)/usr/share
