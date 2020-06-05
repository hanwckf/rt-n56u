SRC_NAME = simple-obfs
THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME) LIBS="-lpthread -lm" LDFLAGS="-Wl, -L$(STAGEDIR)/lib" CFLAGS="-I$(STAGEDIR)/include" 

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		git clone https://github.com/shadowsocks/simple-obfs.git ; \
		cd $(SRC_NAME) ; \
		git submodule update --init --recursive ; \
	fi )
	
config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./autogen.sh ; \
	./configure \
		--prefix=/usr \
		--disable-ssp \
		--disable-assert \
		--disable-documentation \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/obfs-server /usr/bin/obfs-server
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/src/obfs-local /usr/bin/obfs-local
