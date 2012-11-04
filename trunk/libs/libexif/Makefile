SRC_NAME=libexif-0.6.20

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi ) 

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix=/ \
		$(if $(CONFIG_LIB_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(CONFIG_LIB_BUILD_SHARED),--enable-shared,--disable-shared) \
		--disable-nls \
		--disable-docs \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	if [ $(CONFIG_LIB_BUILD_SHARED) = "y" ] ; then \
		cp -fP $(DESTDIR)/lib/libexif.so* $(ROMFSDIR)/lib ; \
	fi
