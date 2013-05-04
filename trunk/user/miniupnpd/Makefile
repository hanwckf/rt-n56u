SRC_NAME=miniupnpd-1.8.x

all:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME)

clean:
	$(MAKE) -f Makefile.linux -C $(SRC_NAME) clean
	rm -f miniupnpd

romfs:
	cp $(SRC_NAME)/miniupnpd .
	$(ROMFSINST) /usr/bin/miniupnpd

