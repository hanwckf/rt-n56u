SRC_NAME=libvorbis-1.3.6
SRC_URL=http://downloads.xiph.org/releases/vorbis/$(SRC_NAME).tar.xz

all: download_test extract_test config_test
	$(MAKE) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.xz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.xz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xJf $(SRC_NAME).tar.xz; \
		patch -d $(SRC_NAME) -p1 -i ../disable-doc-test.patch; \
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
		--with-ogg-libraries=$(DESTDIR)/lib \
		--disable-oggtest \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	rm -f config_done
	rm -rf $(SRC_NAME)

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:


