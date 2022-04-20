THISDIR=$(shell pwd)

PPPD_DIR=$(ROOTDIR)/user/nginx

CFLAGS  += -O2
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

SRC_VER = 1.20.2
SRC_NAME = nginx-$(SRC_VER)
SRC_FILE = $(SRC_NAME).tar.gz
SRC_URL := http://nginx.org/download/$(SRC_FILE)

all: download_test extra_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_FILE) $(SRC_URL); \
	fi )

extra_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xf $(SRC_FILE); \
		sed -i 's/if \/bin\/sh/if \(echo march \| grep march\) \|\| \/bin\/sh/' $(SRC_NAME)/auto/feature ; \
		sed -i 's/if \[ -x/if \(echo march \| grep march\)\; then ngx_size=4; elif \[ -x/' $(SRC_NAME)/auto/types/sizeof ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure --without-http_rewrite_module --without-http_gzip_module --prefix=/tmp/var --conf-path=/tmp/var/nginx.conf --pid-path=/tmp/var/nginx.pid --error-log-path=/tmp/var/nginx.log --http-log-path=/tmp/var/access.log; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/objs/nginx /usr/bin/nginx
	$(ROMFSINST) -p +x $(THISDIR)/nginx.sh /usr/bin/nginx.sh
