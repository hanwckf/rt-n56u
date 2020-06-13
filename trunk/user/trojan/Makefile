SRC_NAME = trojan-1.16.0
THISDIR = $(shell pwd)
CFLAGS := -O3 -ffunction-sections -fdata-sections
CXXFLAGS += -std=c++11
CXXFLAGS := $(filter-out -O%,$(CXXFLAGS)) -O3
CXXFLAGS := -O3 -ffunction-sections -fdata-sections
LDFLAGS := -Wl,--gc-sections

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)/build
	
extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
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
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/build/trojan /usr/bin/trojan
	cp -fP $(CONFIG_CROSS_COMPILER_ROOT)/mipsel-linux-uclibc/sysroot/lib/libstdc++.so* $(ROMFSDIR)/lib
