SRC_NAME = napt66-v1.1
THISDIR = $(shell pwd)

all:
	$(MAKE) -C $(SRC_NAME)

romfs:
	chmod -R +x scripts/
	${ROMFSINST} $(THISDIR)/scripts/ /bin/

clean:
	$(MAKE) -C $(SRC_NAME) clean
