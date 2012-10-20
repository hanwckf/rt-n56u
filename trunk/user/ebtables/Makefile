SRC_NAME=ebtables-2.0.10-4

all:
	$(MAKE) -C $(SRC_NAME) static KERNEL_INCLUDES=$(KERNEL_HEADERS_PATH)

clean:
	rm -f ebtables
	$(MAKE) -C $(SRC_NAME) clean

romfs:
	cp $(SRC_NAME)/static ./ebtables
	$(ROMFSINST) /bin/ebtables
