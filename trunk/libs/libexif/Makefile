SRC_NAME=libexif-0.6.21
SRC_URL=https://sourceforge.net/projects/libexif/files/libexif/0.6.21/libexif-0.6.21.tar.bz2

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.bz2 ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.bz2 $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar jxf $(SRC_NAME).tar.bz2; \
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
		--prefix=$(STAGEDIR) \
		--enable-static \
		--disable-shared \
		--disable-nls \
		--disable-docs \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	rm -f config_done
	rm -rf $(SRC_NAME)

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:

