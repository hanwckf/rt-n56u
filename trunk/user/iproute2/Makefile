SRC_NAME=iproute2-2.6.24

all:
	$(MAKE) -C $(SRC_NAME)

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -f tc

romfs:
	cp $(SRC_NAME)/tc/tc .
	$(ROMFSINST) /bin/tc

