SRC_NAME = scutclient-upstream
ARCHIVE_NAME = scutclient-upstream-ca26d9d
#https://github.com/hanwckf/scutclient.git
THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)/build

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		unzip -qq $(ARCHIVE_NAME).zip; \
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
	chmod -R +x ./scripts
	${ROMFSINST} $(THISDIR)/scripts/ /bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/build/scutclient /usr/bin/bin_scutclient
