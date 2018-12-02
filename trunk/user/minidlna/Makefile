SRC_NAME=minidlna-1.2.1
SRC_URL=https://sourceforge.net/projects/minidlna/files/minidlna/1.2.1/minidlna-1.2.1.tar.gz 

DLNA_CFLAGS  += -ffunction-sections -fdata-sections
DLNA_CFLAGS  += -Os -Wall -g -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
DLNA_CFLAGS  += -I$(STAGEDIR)/include

DLNA_LDFLAGS += -Wl,--gc-sections
DLNA_LDFLAGS += -L$(STAGEDIR)/lib

DLNA_LIBS += -ldl -lm -lpthread -lz -lexif -ljpeg -lsqlite3 -lid3tag -lFLAC -lvorbis -logg -lavformat -lavcodec -lavutil

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xzf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../minidlna-orig.patch; \
		$(SRC_NAME)/autogen.sh; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	ac_cv_lib_avformat_av_open_input_file=yes \
	CFLAGS="$(DLNA_CFLAGS)" LDFLAGS="$(DLNA_LDFLAGS)" LIBS="$(DLNA_LIBS)" \
	./configure \
		--prefix=/usr \
		--disable-nls \
		--enable-asus \
		--enable-tivo \
		--with-libiconv-prefix=$(STAGEDIR) \
		--with-log-path=/mnt/minidlna \
		--with-db-path=/mnt/minidlna \
		--with-os-name="Linux" \
		--with-os-version="Linux/Cross_compiled" \
		--with-os-url="http://www.kernel.org/" \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	rm -f config_done
	rm -rf $(SRC_NAME)

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/minidlnad /usr/bin/minidlnad

