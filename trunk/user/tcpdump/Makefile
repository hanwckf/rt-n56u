SRC_NAME=tcpdump-4.4.0

CFLAGS += -Os -ffunction-sections -fdata-sections -DTCPDUMP_MINI
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
	ac_cv_linux_vers=3 \
	ac_cv_lib_rpc_main=no \
	ac_cv_path_PCAP_CONFIG="" \
	td_cv_buggygetaddrinfo=no \
	./configure \
		--prefix=/usr \
		--without-crypto \
		--without-smi \
		--disable-smb \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

build:
	$(MAKE) -C $(SRC_NAME) TCPDUMP_MINI=1

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done
	rm -f tcpdump

romfs:
	cp $(SRC_NAME)/tcpdump .
	$(STRIP) tcpdump
	$(ROMFSINST) /usr/sbin/tcpdump

