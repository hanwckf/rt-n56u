# source from http://osp.avm.de/fritzbox/fritzbox-7560/source-files-FRITZ.Box_7560-07.11.tar

SRC_NAME = antfs-07.11

all:
ifeq ($(CONFIG_WITHOUT_KERNEL),y)
	@echo "antfs would not build!"
else
	$(MAKE) -C $(SRC_NAME)
	cp -f $(SRC_NAME)/antfs.ko .
endif

romfs:

clean:
	$(MAKE) -C $(SRC_NAME) clean
	rm -f *.ko

