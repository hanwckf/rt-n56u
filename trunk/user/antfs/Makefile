# source from http://osp.avm.de/fritzbox/fritzbox-7560/source-files-FRITZ.Box_7560-07.11.tar
# 07.19  from http://osp.avm.de/fritzbox/fritzbox-7590/source-files-FRITZ.Box_7590-07.19.tar.gz
SRC_NAME = antfs-07.19

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


