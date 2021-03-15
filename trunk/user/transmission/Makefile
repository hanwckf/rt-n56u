SRC_NAME = transmission-3.00
SRC_URL = https://github.com/transmission/transmission/releases/download/3.00/transmission-3.00.tar.xz
CFLAGS   += -O2 -ffunction-sections -fdata-sections -D_LARGEFILE64_SOURCE -D_LFS64_LARGEFILE -Wno-implicit-fallthrough
LDFLAGS  += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build && \
	$(MAKE) -C $(SRC_NAME)/build install DESTDIR=$(THISDIR)/install

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar -xf $(SRC_NAME).tar.xz; \
		patch -p1 -d $(SRC_NAME) < $(SRC_NAME).patch; \
		mkdir -p $(SRC_NAME)/third-party/miniupnpc/include/miniupnp ; \
		cp -r $(SRC_NAME)/third-party/miniupnpc/*.h $(SRC_NAME)/third-party/miniupnpc/include/miniupnp ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../cross-mipsel-linux.cmake \
		  -DCMAKE_INSTALL_PREFIX=/usr \
		  -DCMAKE_BUILD_TYPE=Release \
		  -DENABLE_CLI=OFF \
		  -DENABLE_DAEMON=ON \
		  -DENABLE_GTK=OFF \
		  -DENABLE_LIGHTWEIGHT=ON \
		  -DENABLE_NLS=OFF \
		  -DENABLE_QT=OFF \
		  -DENABLE_TESTS=OFF \
		  -DINSTALL_DOC=OFF \
		  -DUSE_SYSTEM_MINIUPNPC=OFF \
		  -DUSE_SYSTEM_NATPMP=OFF \
		  -DUSE_SYSTEM_UTP=OFF \
		  -DWITH_SYSTEMD=OFF \
		  -DWITH_INOTIFY=ON \
		  -DWITH_CRYPTO=openssl \
		  .. \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done
	rm -rf install

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION_WEB_CONTROL),y)
	mkdir -p $(INSTALLDIR)/usr/share/transmission
	cp -rf $(THISDIR)/transmission-control/web $(INSTALLDIR)/usr/share/transmission
else
	cp -rf $(THISDIR)/install/usr/share/transmission $(INSTALLDIR)/usr/share
	rm -f $(INSTALLDIR)/usr/share/transmission/web/LICENSE
endif
	$(ROMFSINST) $(THISDIR)/install/usr/bin/transmission-daemon /usr/bin/transmission-daemon
	$(ROMFSINST) -p +x /usr/bin/transmission.sh
