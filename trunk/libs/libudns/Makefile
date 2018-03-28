SRC_NAME=udns-0.4
SRC_URL=http://www.corpit.ru/mjt/udns/udns-0.4.tar.gz

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xf $(SRC_NAME).tar.gz ; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	patch -s -r - -N -p1 <../100-cross-compile-dirty-fix.patch ; \
	./configure \
		--enable-cross_compile ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	cp -f $(SRC_NAME)/udns.h $(STAGEDIR)/include
	cp -f $(SRC_NAME)/*.a $(STAGEDIR)/lib

romfs:
	
