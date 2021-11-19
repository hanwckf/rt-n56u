SRC_NAME = htop
SRC_VERSION = 3.1.1
BIN_SOURCE = $(SRC_NAME)-$(SRC_VERSION)
SRC_URL = https://github.com/htop-dev/htop/archive/refs/tags/$(SRC_VERSION).tar.gz

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(BIN_SOURCE)

download_test:
	( if [ ! -f $(BIN_SOURCE).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(BIN_SOURCE).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(BIN_SOURCE) ]; then \
		tar -xzf $(BIN_SOURCE).tar.gz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(BIN_SOURCE) ; \
	./autogen.sh ; \
	./configure \
		--prefix=/usr \
		--enable-unicode \
		HTOP_NCURSESW_CONFIG_SCRIPT="$(STAGEDIR)/bin/ncursesw6-config" \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(BIN_SOURCE)/Makefile ] ; then \
		$(MAKE) -C $(BIN_SOURCE) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(BIN_SOURCE)/htop /usr/bin/htop
