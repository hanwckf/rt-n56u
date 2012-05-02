SRC_NAME="inadyn-1.96.3"

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
	ac_cv_func_malloc_0_nonnull="yes" ac_cv_func_realloc_0_nonnull="yes" \
	use_cache_file="yes" \
	./configure \
		--prefix= \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -f inadyn
	rm -f config_done

romfs:
	cp $(SRC_NAME)/src/inadyn .
	$(STRIP) inadyn
	$(ROMFSINST) /bin/inadyn
	
