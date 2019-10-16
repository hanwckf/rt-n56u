SRC_NAME=iptables-1.4.16.3

CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/*.* $(THISDIR)/$(SRC_NAME)/extensions
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix= \
		--enable-static \
		--disable-shared \
		--with-xtlibdir=/usr/lib/xtables \
		--with-kernel=$(ROOTDIR)/$(LINUXDIR) \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	mkdir -p $(ROMFSDIR)/usr/lib/xtables
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/iptables/xtables-multi /bin/xtables-multi
	$(ROMFSINST) -s xtables-multi /bin/iptables
	$(ROMFSINST) -s xtables-multi /bin/iptables-restore
	$(ROMFSINST) -s xtables-multi /bin/iptables-save
ifdef CONFIG_IPV6
	$(ROMFSINST) -s xtables-multi /bin/ip6tables
	$(ROMFSINST) -s xtables-multi /bin/ip6tables-restore
	$(ROMFSINST) -s xtables-multi /bin/ip6tables-save
endif
