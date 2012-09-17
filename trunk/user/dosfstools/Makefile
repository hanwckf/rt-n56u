SRC_NAME=dosfstools-3.0.12

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -f dosfsck

romfs:
	cp $(SRC_NAME)/dosfsck .
	$(STRIP) dosfsck
	$(ROMFSINST) /sbin/dosfsck

