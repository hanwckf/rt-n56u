SRC_NAME=flac-1.3.2
SRC_URL=https://ftp.osuosl.org/pub/xiph/releases/flac/$(SRC_NAME).tar.xz

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xJf  $(SRC_NAME).tar.xz ; \
		patch -d $(SRC_NAME) -p1 -i ../flac-orig.patch ; \
		patch -d $(SRC_NAME) -p1 -i ../100-CVE-2017-6888.patch ; \
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
		--prefix=$(STAGEDIR) \
		$(if $(LIBS_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(LIBS_BUILD_SHARED),--enable-shared,--disable-shared) \
		--disable-cpplibs \
		--disable-sse \
		--disable-3dnow \
		--disable-altivec \
		--disable-doxgen-docs \
		--disable-local-xmms-plugin \
		--disable-xmms-plugin \
		--disable-ogg \
		--disable-oggtest \
		--disable-debug \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:
ifeq ($(LIBS_BUILD_SHARED),y)
	cp -fP $(DESTDIR)/lib/libFLAC.so* $(ROMFSDIR)/lib ;
endif
