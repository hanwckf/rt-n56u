SRC_NAME="igmpproxy-0.1"

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
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f igmpproxy
	rm -f config_done

romfs:
	cp $(SRC_NAME)/src/igmpproxy .
	$(STRIP) igmpproxy
	$(ROMFSINST) /bin/igmpproxy
