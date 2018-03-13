SRC_NAME=shadowsocksr-libev-master
ARCHIVE_NAME=shadowsocksr-libev-master-e2373d7
#https://github.com/shadowsocksr-backup/shadowsocksr-libev.git
THISDIR = $(shell pwd)

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS_MBEDTLS),y)
	ss_crypto_lib=mbedtls
else
	ss_crypto_lib=openssl
endif

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(ARCHIVE_NAME).zip ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix=/usr \
		--disable-documentation \
		--disable-ssp \
		--disable-assert \
		ac_cv_prog_PCRE_CONFIG="$(STAGEDIR)/bin/pcre-config" \
		--with-crypto-library=$(ss_crypto_lib) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	chmod -R +x scripts/
	$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/ss-redir /usr/bin/ss-redir
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/ss-local /usr/bin/ss-local
