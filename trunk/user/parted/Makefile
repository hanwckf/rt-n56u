SRC_NAME=parted-3.2
SRC_URL=https://ftp.gnu.org/gnu/parted/$(SRC_NAME).tar.xz 

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xJf $(SRC_NAME).tar.xz; \
		patch -d $(SRC_NAME) -p1 -i ../$(SRC_NAME).patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	PKG_CONFIG_PATH="$(STAGEDIR)/lib/pkgconfig" \
	LIBS="-liconv" \
	./configure \
		--prefix=/usr \
		--without-readline \
		--disable-shared \
		--disable-device-mapper \
		--disable-nls \
		--disable-debug \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/parted/parted /sbin/parted

