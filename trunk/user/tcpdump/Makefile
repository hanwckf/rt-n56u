SRC_NAME=tcpdump-4.9.2
SRC_URL=https://www.tcpdump.org/release/$(SRC_NAME).tar.gz

CFLAGS += -Os -ffunction-sections -fdata-sections -fvisibility=hidden -DTCPDUMP_MINI
LDFLAGS += -Wl,--gc-sections

all: download_test extract_test config_test build

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../100-tcpdump_mini.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	ac_cv_linux_vers=3 \
	ac_cv_header_rpc_rpcent_h=no \
	ac_cv_lib_rpc_main=no \
	ac_cv_path_PCAP_CONFIG="$(STAGEDIR)/bin/pcap-config" \
	td_cv_buggygetaddrinfo=no \
	./configure \
		--prefix=/usr \
		--without-crypto \
		--without-smi \
		--without-cap-ng \
		--disable-smb \
		$(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

build:
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) TCPDUMP_MINI=1

clean:
	rm -f config_done
	rm -f tcpdump
	rm -rf $(SRC_NAME)

romfs:
	cp $(SRC_NAME)/tcpdump .
	$(STRIP) tcpdump
	$(ROMFSINST) /usr/sbin/tcpdump

