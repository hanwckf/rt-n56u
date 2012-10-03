SRC_NAME=radvd-1.8.5

SHAREDIR = $(ROOTDIR)/user/shared

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

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
		--prefix= \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -f radvd
	rm -f config_done

romfs:
	cp $(SRC_NAME)/radvd .
	$(STRIP) radvd
	$(ROMFSINST) /usr/sbin/radvd

