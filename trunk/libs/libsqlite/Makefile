SRC_NAME=sqlite-autoconf-3070701

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
	./configure \
		--prefix=$(STAGEDIR) \
		$(if $(LIBS_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(LIBS_BUILD_SHARED),--enable-shared,--disable-shared) \
		--disable-readline \
		--disable-dynamic-extensions \
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
	cp -fP $(DESTDIR)/lib/libsqlite3.so* $(ROMFSDIR)/lib ;
endif

