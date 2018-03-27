SRC_NAME = scutclient-upstream
ARCHIVE_NAME = scutclient-upstream-2b45c37
#https://github.com/hanwckf/scutclient.git
THISDIR = $(shell pwd)

all: extract_test config_test
	$(MAKE) -C $(SRC_NAME)

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		unzip -qq $(ARCHIVE_NAME).zip; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	autoreconf -fi ; \
	./configure \
		--prefix=/usr \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
	chmod -R +x ./scripts
	${ROMFSINST} $(THISDIR)/scripts/ /bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/scutclient /usr/bin/bin_scutclient
