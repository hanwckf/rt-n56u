SRC_NAME = skipdbv2
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
	cmake -DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_EXE_LINKER_FLAGS="-L$(STAGEDIR)/lib -lev"; \
	)

clean:
	$(MAKE) -C skipdbv2 -f Makefile clean
	rm -rf skipdbv2/bin
	rm -rf skipdbv2/lib
	rm -rf skipdbv2/CMakeFiles
	rm -f skipdbv2/CMakeCache.txt
	rm -f skipdbv2/cmake_install.cmake
	rm -f skipdbv2/Makefile
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/bin/skipd /usr/sbin/skipd
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/bin/dbus /usr/sbin/dbus
	
