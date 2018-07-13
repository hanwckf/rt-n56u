SRC_NAME = rtl-sdr-0.5.4
SRC_URL = https://github.com/osmocom/rtl-sdr/archive/0.5.4.zip
THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).zip $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		unzip $(SRC_NAME).zip; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	autoreconf -fi ; \
	./configure \
		--disable-shared \
		--enable-static \
		--prefix=/usr \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_adsb /usr/bin/rtl_adsb
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_eeprom /usr/bin/rtl_eeprom
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_fm /usr/bin/rtl_fm
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_power /usr/bin/rtl_power
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_sdr /usr/bin/rtl_sdr
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_tcp /usr/bin/rtl_tcp
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/rtl_test /usr/bin/rtl_test
