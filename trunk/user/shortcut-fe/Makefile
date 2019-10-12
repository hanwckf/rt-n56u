SRC_NAME = shortcut-fe
THISDIR = $(shell pwd)

all:
ifeq ($(CONFIG_WITHOUT_KERNEL),y)
	@echo "shortcut-fe would not build!"
else
	$(MAKE) -C $(SRC_NAME)
endif

romfs:

clean:
	$(MAKE) -C $(SRC_NAME) clean
