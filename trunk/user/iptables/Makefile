SRC_NAME=iptables-1.8.4

CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xvf $(SRC_NAME).tar.bz2 ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./autogen.sh ; \
	./configure \
		--prefix= \
		--enable-static \
		--disable-shared \
		--disable-nftables \
		--with-xtlibdir=/usr/lib/xtables \
		--with-xt-lock-name=/var/run/xtables.lock \
		--with-kernel=$(ROOTDIR)/$(LINUXDIR) \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	rm -rf $(SRC_NAME)
	rm -f config_done
romfs:
	mkdir -p $(ROMFSDIR)/usr/lib/xtables
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/iptables/xtables-legacy-multi /bin/xtables-legacy-multi
	$(ROMFSINST) -s xtables-legacy-multi /bin/iptables
	$(ROMFSINST) -s xtables-legacy-multi /bin/iptables-restore
	$(ROMFSINST) -s xtables-legacy-multi /bin/iptables-save
ifdef CONFIG_IPV6
	$(ROMFSINST) -s xtables-legacy-multi /bin/ip6tables
	$(ROMFSINST) -s xtables-legacy-multi /bin/ip6tables-restore
	$(ROMFSINST) -s xtables-legacy-multi /bin/ip6tables-save
endif
