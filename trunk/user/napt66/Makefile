SRC_NAME = napt66-v1.1
THISDIR = $(shell pwd)

all:
ifeq ($(CONFIG_WITHOUT_KERNEL),y)
	@echo "napt66 build done"
else
	$(MAKE) -C $(SRC_NAME)
endif

romfs:
	chmod -R +x scripts/
	${ROMFSINST} $(THISDIR)/scripts/ /bin/

clean:
	$(MAKE) -C $(SRC_NAME) clean
