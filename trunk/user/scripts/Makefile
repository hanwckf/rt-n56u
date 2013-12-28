SCRIPTS = $(notdir $(wildcard *.sh))

all:

clean:

romfs:
	for i in $(SCRIPTS) ; do \
		$(ROMFSINST) $$i /sbin/$$i ; \
	done
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
