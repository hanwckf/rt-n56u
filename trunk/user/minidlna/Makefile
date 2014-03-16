SRC_NAME=minidlna-1.x.x

DLNA_CFLAGS  += -ffunction-sections -fdata-sections
DLNA_CFLAGS  += -Os -Wall -g -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
DLNA_CFLAGS  += -I$(STAGEDIR)/include

DLNA_LDFLAGS += -Wl,--gc-sections
DLNA_LDFLAGS += -L$(STAGEDIR)/lib

DLNA_LIBS += -ldl -lm -lpthread -lz -lexif -ljpeg -lsqlite3 -lid3tag -lFLAC -lvorbis -logg -lavformat -lavcodec -lavutil

THISDIR = $(shell pwd)

all: config_test
	$(MAKE) -C $(SRC_NAME)

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
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/minidlnad /usr/bin/minidlnad

