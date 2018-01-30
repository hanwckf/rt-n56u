SRC_NAME = nano-2.9.3
SRC_URL = https://www.nano-editor.org/dist/v2.9/nano-2.9.3.tar.gz
THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar zxf $(SRC_NAME).tar.gz; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix=/usr \
		--disable-browser \
		--enable-tiny \
		--disable-utf8 \
		--without-slang \
		LDFLAGS="-L$(STAGEDIR)/lib" \
		CFLAGS="-I$(STAGEDIR)/include" \
		NCURSES_CFLAGS="-D_GNU_SOURCE -I$(STAGEDIR)/include" \
		NCURSES_LIBS="-L$(STAGEDIR)/lib -Wl,-rpath,$(STAGEDIR)/lib -lncurses" \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/src/nano /usr/bin/nano
