all:
	( if [ ! -d $(MERGE_ROOTFS) ] ; then \
		@echo "unpacked rootfs not found!" ; \
		exit 1 ; \
	fi)
	mkdir -p $(CONFIG_FIRMWARE_PRODUCT_ID)
	cp -fP $(MERGE_ROOTFS)/lib/libiw.so* $(CONFIG_FIRMWARE_PRODUCT_ID)

clean:
	rm -rf $(CONFIG_FIRMWARE_PRODUCT_ID)

install:
	@echo "libiw install done"

romfs:
	cp -fP $(CONFIG_FIRMWARE_PRODUCT_ID)/libiw.so* $(ROMFSDIR)/lib
	cp -f $(MERGE_ROOTFS)/bin/iwpriv $(ROMFSDIR)/bin/iwpriv
