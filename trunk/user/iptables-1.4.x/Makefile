SRC_NAME=iptables-1.4.15

CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

ipt_blist = libip6t_ah libip6t_eui64 libip6t_frag libip6t_hbh libip6t_hl libip6t_ipv6header libip6t_mh libip6t_dst \
            libipt_ah libipt_CLUSTERIP libipt_ECN libipt_MIRROR libipt_realm libipt_SAME libipt_ULOG libipt_unclean \
            libxt_addrtype libxt_AUDIT libxt_CHECKSUM libxt_cluster libxt_comment libxt_CONNSECMARK libxt_cpu libxt_CT \
            libxt_dccp libxt_devgroup libxt_dscp libxt_DSCP libxt_ecn libxt_esp libxt_hashlimit libxt_helper libxt_HMARK \
            libxt_IDLETIMER libxt_LED libxt_nfacct libxt_NFLOG libxt_NFQUEUE libxt_NOTRACK libxt_osf libxt_owner libxt_physdev \
            libxt_pkttype libxt_policy libxt_quota libxt_rateest libxt_RATEEST libxt_rpfilter libxt_sctp libxt_SECMARK libxt_set \
            libxt_SET libxt_socket libxt_statistic libxt_TCPOPTSTRIP libxt_TEE libxt_TPROXY libxt_TRACE

STG_DIR = $(ROOTDIR)/stage

all:	config_test
	$(MAKE) -C $(SRC_NAME) && \
	$(MAKE) -C $(SRC_NAME) install-strip DESTDIR=$(STG_DIR) && \
	$(STRIP) $(STG_DIR)/usr/lib/xtables/*.so ;

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix= \
		--with-pic \
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
	rm -rf $(STG_DIR)/usr/lib/xtables

romfs:
	mkdir -p $(ROMFSDIR)/usr/lib/xtables
	cp -fP $(STG_DIR)/sbin/* $(ROMFSDIR)/bin
	cp -fP $(STG_DIR)/lib/libiptc.so* $(ROMFSDIR)/lib
	cp -fP $(STG_DIR)/lib/libip4tc.so* $(ROMFSDIR)/lib
ifdef CONFIG_IPV6
	cp -fP $(STG_DIR)/lib/libip6tc.so* $(ROMFSDIR)/lib
endif
	cp -fP $(STG_DIR)/lib/libxtables.so* $(ROMFSDIR)/lib
	cp -fP $(STG_DIR)/usr/lib/xtables/*.so $(ROMFSDIR)/usr/lib/xtables
	for i in $(ipt_blist) ; do \
		rm -f $(ROMFSDIR)/usr/lib/xtables/$$i.so ; \
	done

