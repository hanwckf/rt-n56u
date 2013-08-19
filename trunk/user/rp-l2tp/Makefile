SRC_NAME=rp-l2tp-0.5-rtn

CFLAGS   =-O2 -fomit-frame-pointer -pipe -Wall -Wno-trigraphs -Wno-pointer-sign -Wno-strict-aliasing -I$(KERNEL_HEADERS_PATH)
CFLAGS  +=-DPPPOL2TP_V1
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all: config_test build

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
		--sysconfdir=/etc \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

build:
	$(MAKE) -C $(SRC_NAME)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done
	rm -f l2tpd
	rm -f l2tp-control
	rm -f cmd.so
	rm -f sync-pppd.so

romfs:
	cp $(SRC_NAME)/l2tpd .
	cp $(SRC_NAME)/handlers/l2tp-control .
	cp $(SRC_NAME)/handlers/cmd.so .
	cp $(SRC_NAME)/handlers/sync-pppd.so .
	$(STRIP) l2tpd
	$(STRIP) l2tp-control
	$(STRIP) cmd.so
	$(STRIP) sync-pppd.so
	mkdir -p $(INSTALLDIR)/usr/lib/l2tp
	$(ROMFSINST) /usr/sbin/l2tpd
	$(ROMFSINST) /usr/sbin/l2tp-control
	$(ROMFSINST) /usr/lib/l2tp/cmd.so
	$(ROMFSINST) /usr/lib/l2tp/sync-pppd.so
