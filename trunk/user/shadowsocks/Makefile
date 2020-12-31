THISDIR = $(shell pwd)

SRC_SS_NAME=shadowsocks-libev-3.3.5
#https://github.com/shadowsocks/shadowsocks-libev.git

SRC_SSR_NAME=shadowsocksr-libev-master
ARCHIVE_SSR_NAME=shadowsocksr-libev-master-e2373d7
#https://github.com/shadowsocksr-backup/shadowsocksr-libev.git

GFWLIST_NAME=dnsmasq_gfwlist.conf
GFWLIST_URL=https://cokebar.github.io/gfwlist2dnsmasq/$(GFWLIST_NAME)
GFWLIST_DIR=gfwlist

all: gfwlist_dl extract_test config_test
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_SSR_NAME)
endif
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_SS_NAME)

gfwlist_dl:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
	mkdir -p $(GFWLIST_DIR)
	( if [ ! -f $(GFWLIST_DIR)/.finish ]; then \
		cd $(GFWLIST_DIR) && rm -f * && \
		wget -q -t5 --timeout=20 --no-check-certificate $(GFWLIST_URL) && \
		tar jcf gfwlist.bz2 $(GFWLIST_NAME) && \
		touch .finish ; \
	fi )
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
extract_test: extract_ssr_test extract_ss_test
else
extract_test: extract_ss_test
endif

extract_ssr_test:
	( if [ ! -d $(SRC_SSR_NAME) ]; then \
		unzip -qq $(ARCHIVE_SSR_NAME).zip ; \
	fi )

extract_ss_test:
	( if [ ! -d $(SRC_SS_NAME) ]; then \
		tar xf $(SRC_SS_NAME).tar.gz ; \
	fi )

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
config_test: config_ssr config_ss
else
config_test: config_ss
endif

config_ss:
	( if [ -f ./ss_config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure_ss && touch ss_config_done; \
	fi )

config_ssr:
	( if [ -f ./ssr_config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure_ssr && touch ssr_config_done; \
	fi )

configure_ssr:
	( cd $(SRC_SSR_NAME) ; \
	ac_cv_prog_PCRE_CONFIG="$(STAGEDIR)/bin/pcre-config" \
	./configure \
		--prefix=/usr \
		--disable-documentation \
		--disable-ssp \
		--disable-assert \
		--enable-system-shared-lib \
		--with-crypto-library=mbedtls \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

configure_ss:
	( cd $(SRC_SS_NAME) ; \
	ac_cv_prog_PCRE_CONFIG="$(STAGEDIR)/bin/pcre-config" \
	./configure \
		--prefix=/usr \
		--disable-documentation \
		--disable-ssp \
		--disable-assert \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean: clean_ssr clean_ss
	rm -f ssr_config_done ss_config_done
	rm -rf $(GFWLIST_DIR)

clean_ssr:
	( if [ -f $(SRC_SSR_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_SSR_NAME) clean ; \
	fi )

clean_ss:
	( if [ -f $(SRC_SS_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_SS_NAME) distclean ; \
	fi )

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
	chmod -R +x scripts/
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) $(THISDIR)/gfwlist/gfwlist.bz2 /etc_ro/gfwlist.bz2
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SSR_NAME)/src/ss-redir /usr/bin/ssr-redir
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SSR_NAME)/src/ss-local /usr/bin/ssr-local
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SS_NAME)/src/ss-redir /usr/bin/ss-orig-redir
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SS_NAME)/src/ss-tunnel /usr/bin/ss-orig-tunnel
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SS_NAME)/src/ss-local /usr/bin/ss-orig-local
	ln -sf /var/ss-redir $(ROMFSDIR)/usr/bin/ss-redir
	ln -sf /var/ss-tunnel $(ROMFSDIR)/usr/bin/ss-local
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SSSERVER),y)
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_SS_NAME)/src/ss-server /usr/bin/ss-server
endif
