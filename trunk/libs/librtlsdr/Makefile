SRC_NAME=librtlsdr-0.6.0
SRC_URL=https://github.com/steve-m/librtlsdr/archive/v0.6.0.zip

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build

download_test:
	( if [ ! -f $(SRC_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).zip $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip -qq $(SRC_NAME).zip; \
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
	-DCMAKE_INSTALL_PREFIX=$(STAGEDIR) \
	-DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake \
	.. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME)/build install DESTDIR=""

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_RTL_SDR),y)
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_adsb /usr/bin/rtl_adsb
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_eeprom /usr/bin/rtl_eeprom
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_fm /usr/bin/rtl_fm
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_power /usr/bin/rtl_power
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_sdr /usr/bin/rtl_sdr
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_tcp /usr/bin/rtl_tcp
	$(ROMFSINST) -d $(DESTDIR)/bin/rtl_test /usr/bin/rtl_test
endif
	cp -fP $(DESTDIR)/lib/librtlsdr.so* $(ROMFSDIR)/lib
