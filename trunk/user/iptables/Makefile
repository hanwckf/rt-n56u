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
ifeq ($(CONFIG_FIRMWARE_INCLUDE_IPSET),y)
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_set.* $(THISDIR)/$(SRC_NAME)/extensions
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_SET.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_set.*
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_SET.*
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_XFRM),y)
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_policy.* $(THISDIR)/$(SRC_NAME)/extensions
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libipt_ah.* $(THISDIR)/$(SRC_NAME)/extensions
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libip6t_ah.* $(THISDIR)/$(SRC_NAME)/extensions
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_esp.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_policy.*
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libipt_ah.*
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libip6t_ah.*
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_esp.*
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_QOS),y)
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_CLASSIFY.* $(THISDIR)/$(SRC_NAME)/extensions
ifeq ($(CONFIG_FIRMWARE_INCLUDE_IMQ),y)
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_IMQ.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_IMQ.*
endif
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_CLASSIFY.*
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_IMQ.*
endif
ifdef CONFIG_NETFILTER_XT_MATCH_CONNTRACK
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_conntrack.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_conntrack.*
endif
ifdef CONFIG_NETFILTER_XT_MATCH_PHYSDEV
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_physdev.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_physdev.*
endif
ifdef CONFIG_NETFILTER_XT_MATCH_STRING
	cp -f $(THISDIR)/$(SRC_NAME)/extensions/disabled/libxt_string.* $(THISDIR)/$(SRC_NAME)/extensions
else
	rm -f $(THISDIR)/$(SRC_NAME)/extensions/libxt_string.*
endif
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
