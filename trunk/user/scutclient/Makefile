# https://github.com/scutclient/scutclient.git
SRC_NAME := scutclient-master
SRC_URL = https://github.com/scutclient/scutclient/archive/refs/heads/master.zip
THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)/build

download_test: 
	( if [ ! -f $(THISDIR)/$(SRC_NAME).zip ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).zip $(SRC_URL); \
	fi )
	
extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
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
		cmake -DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake .. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

romfs:
	${ROMFSINST} $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/build/scutclient /usr/bin/bin_scutclient
