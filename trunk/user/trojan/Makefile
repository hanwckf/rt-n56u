THISDIR = $(shell pwd)
SRC_NAME = trojan
SRC_VERSION := 1.16.0
SRC_DIR := $(SRC_NAME)-$(SRC_VERSION)
SRC_URL := https://github.com/trojan-gfw/trojan/archive/v$(SRC_VERSION).tar.gz
CFLAGS := -O3 -ffunction-sections -fdata-sections
CXXFLAGS += -std=c++11
CXXFLAGS := $(filter-out -O%,$(CXXFLAGS)) -O3
CXXFLAGS := -O3 -ffunction-sections -fdata-sections
LDFLAGS := -Wl,--gc-sections

all: download_src extract_test config_test
	$(MAKE) -C $(SRC_DIR)/build
	
download_src:
	( if [ ! -f $(THISDIR)/$(SRC_DIR).tar.gz ]; then \
	curl --create-dirs -L $(SRC_URL) -o $(THISDIR)/$(SRC_DIR).tar.gz ; \
	fi )	

extract_test:
	( if [ ! -d $(SRC_DIR) ]; then \
		rm -f config_done ; \
		tar zxf $(SRC_DIR).tar.gz ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_DIR)/build && cd $(SRC_DIR)/build ; \
		CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" \
		cmake -DCMAKE_TOOLCHAIN_FILE=../../cross-linux.cmake \
			-DENABLE_MYSQL=OFF \
			-DENABLE_NAT=ON \
			-DENABLE_REUSE_PORT=ON \
			-DENABLE_SSL_KEYLOG=ON \
			-DENABLE_TLS13_CIPHERSUITES=ON \
			-DFORCE_TCP_FASTOPEN=OFF \
			-DSYSTEMD_SERVICE=OFF \
			-DOPENSSL_USE_STATIC_LIBS=FALSE \
			-DBoost_DEBUG=OFF \
			-DBoost_NO_BOOST_CMAKE=ON \
			.. ; \
	)

clean:
	rm -f config_done
	if [ -f $(SRC_DIR)/Makefile ] ; then \
		$(MAKE) -C $(SRC_DIR) distclean ; \
	fi ; \

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_DIR)/build/trojan /usr/bin/trojan
	cp -fP $(CONFIG_CROSS_COMPILER_ROOT)/mipsel-linux-uclibc/sysroot/lib/libstdc++.so* $(ROMFSDIR)/lib
