SRC_NAME=tcpdump-4.1.1

CFLAGS += -Os -ffunction-sections -fdata-sections -DTCPDUMP_MINI
LDFLAGS += -Wl,--gc-sections

all:	config_test build

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	export ac_cv_linux_vers=2 ; \
	export td_cv_buggygetaddrinfo=no ; \
	./configure \
		--prefix=/usr \
		--without-crypto \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

build:
	$(MAKE) -C $(SRC_NAME) TCPDUMP_MINI=1

clean:
	rm -f config_done
	rm -f tcpdump
	$(MAKE) -C $(SRC_NAME) distclean

romfs:
	cp $(SRC_NAME)/tcpdump .
	$(STRIP) tcpdump
	$(ROMFSINST) /usr/sbin/tcpdump

