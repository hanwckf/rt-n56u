SRC_NAME=mt-daapd-svn

THISDIR = $(shell pwd)

all: config_test
	$(MAKE) -C $(SRC_NAME) && \
	$(MAKE) -C $(SRC_NAME) install-strip DESTDIR=$(THISDIR)/install

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	ac_cv_func_setpgrp_void=yes \
	ac_cv_lib_FLAC_FLAC__metadata_chain_read=yes \
	PKG_CONFIG_PATH=$(STAGEDIR)/lib/pkgconfig \
	./configure \
		--prefix= \
		--enable-iconv \
		--enable-mdns \
		--enable-ffmpeg \
		--enable-flac \
		--enable-sqlite3 \
		--with-sqlite3-includes=$(STAGEDIR)/include \
		--with-sqlite3-libs=$(STAGEDIR)/lib \
		--with-ffmpeg-includes=$(STAGEDIR)/include \
		--with-ffmpeg-libs=$(STAGEDIR)/lib \
		--with-id3tag=$(STAGEDIR) \
		--with-libiconv-prefix=$(STAGEDIR) \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done
	rm -rf install

romfs:
	mkdir -p $(INSTALLDIR)/usr/lib/mt-daapd
	mkdir -p $(INSTALLDIR)/usr/share/mt-daapd
	cp -rf $(THISDIR)/install/share/mt-daapd/admin-root/* $(INSTALLDIR)/usr/share/mt-daapd
	$(ROMFSINST) $(THISDIR)/install/lib/mt-daapd/plugins/out-daap.so /usr/lib/mt-daapd/out-daap.so
	$(ROMFSINST) $(THISDIR)/install/lib/mt-daapd/plugins/rsp.so /usr/lib/mt-daapd/rsp.so
	$(ROMFSINST) $(THISDIR)/install/lib/mt-daapd/plugins/ssc-ffmpeg.so /usr/lib/mt-daapd/ssc-ffmpeg.so
	$(ROMFSINST) $(THISDIR)/install/sbin/mt-daapd /usr/bin/mt-daapd
