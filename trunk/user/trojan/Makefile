SRC_NAME = trojan-1.16.0
SRC_URL = https://github.com/trojan-gfw/trojan/archive/v1.16.0.tar.gz
THISDIR = $(shell pwd)

LDFLAGS += -static-libstdc++
COPTS = -O3 $(filter-out -O%,$(CXXFLAGS))
CFLAGS := $(COPTS)
CXXFLAGS := $(COPTS)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build

download_test:
	( if [ ! -f $(THISDIR)/$(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.gz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
		cmake \
			-DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake \
			-DBoost_USE_STATIC_LIBS=ON \
			-DENABLE_MYSQL=OFF \
			-DENABLE_REUSE_PORT=ON \
			-DSYSTEMD_SERVICE=OFF \
			.. ; \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/build/trojan /usr/bin/trojan

