SRC_NAME = scutclient-3857f08268a0522b17e337217b3da173c4653ba1
#https://github.com/hanwckf/scutclient.git
THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)/build

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
		cmake -DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake .. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

romfs:
	${ROMFSINST} $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/build/scutclient /usr/bin/bin_scutclient
