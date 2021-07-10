THISDIR := $(shell pwd)
NPC_VER := 0.26.10
NPC__URL_BASE := https://github.com/ehang-io/nps/releases/download

NPC_NAME := linux_mipsle_client
NPC_URL := $(NPC__URL_BASE)/v$(NPC_VER)/$(NPC_NAME).tar.gz

all: download_test extra_test
	@echo "npc build done!"

download_test:
	( if [ ! -f $(NPC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(NPC_NAME).tar.gz $(NPC_URL); \
	fi )

extra_test:
	( if [ ! -d $(NPC_NAME) ]; then \
		mkdir $(NPC_NAME); \
		tar xf $(NPC_NAME).tar.gz -C $(NPC_NAME); \
	fi )

clean:
	rm -rf $(NPC_NAME).tar.gz $(NPC_NAME)

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NPC),y)
	$(ROMFSINST) -p +x $(THISDIR)/$(NPC_NAME)/npc /usr/bin/npc
endif
	$(ROMFSINST) -p +x $(THISDIR)/npc.sh /usr/bin/npc.sh
	$(ROMFSINST) /etc_ro/npc_script.sh
