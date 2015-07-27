KERNELZ		= $(IMAGEDIR)/zImage
RAMDISK		= $(IMAGEDIR)/ramdisk
IMAGE		= $(IMAGEDIR)/$(CONFIG_FIRMWARE_PRODUCT_ID)_$(FIRMWARE_KERNEL_VER).$(FIRMWARE_ROOTFS_VER)-$(FIRMWARE_BUILDS_VER).trx
BOARD_DIR	= $(ROOTDIR)/configs/boards/$(CONFIG_FIRMWARE_PRODUCT_ID)
ROMFS_DIR	= $(ROOTDIR)/romfs
MKIMAGE_DIR	:= $(ROOTDIR)/tools/mkimage
CUR_DIR		= $(shell pwd)
COMP		= lzma
ifeq ($(CONFIG_MTD_NAND_RALINK),y)
PART_ALIGN	= 131072
else
PART_ALIGN	= 16
endif
ROMFS_DIRS	= \
	bin \
	dev dev/pts \
	home \
	etc \
	etc_ro etc_ro/ppp/peers etc_ro/ppp/plugins etc_ro/ppp/3g \
		etc_ro/Wireless etc_ro/Wireless \
	lib lib/modules \
	mnt \
	media \
	opt \
	proc \
	sbin \
	sys \
	usr usr/bin usr/sbin usr/lib/pppd usr/codepages \
	tmp \
	var

clean:
	rm -f $(RAMDISK)

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	for i in `ls $(BOARD_DIR)/SingleSKU*.dat` ; do \
		[ -f $$i ] && cp -f $$i $(ROMFSDIR)/etc_ro/Wireless; \
	done
ifeq ($(CONFIG_EXTERNAL_PA_EXTERNAL_LNA),y)
	$(ROMFSINST) MT7620_AP_2T2R-4L_external_LNA_external_PA_V15.bin /etc_ro/Wireless/MT7620_AP_2T2R-4L_V15.bin
else
ifeq ($(CONFIG_INTERNAL_PA_EXTERNAL_LNA),y)
	$(ROMFSINST) MT7620_AP_2T2R-4L_external_LNA_internal_PA_V15.bin /etc_ro/Wireless/MT7620_AP_2T2R-4L_V15.bin
else
	$(ROMFSINST) MT7620_AP_2T2R-4L_internal_LNA_internal_PA_V15.bin /etc_ro/Wireless/MT7620_AP_2T2R-4L_V15.bin
endif
endif
ifdef CONFIG_RT5592_AP
ifeq ($(CONFIG_RT5592_AP_EP_SUPPORT),y)
	$(ROMFSINST) /etc_ro/Wireless/RT5592EP_PCIe_2T2R_5G_V1_4.bin
else
	$(ROMFSINST) /etc_ro/Wireless/RT5592_PCIe_2T2R_V1_7.bin
endif
endif
ifdef CONFIG_MT7610_AP
	$(ROMFSINST) /etc_ro/Wireless/MT7610E-V10-FEM.bin
endif
ifdef CONFIG_MT76X2_AP
	$(ROMFSINST) /etc_ro/Wireless/MT7612E_EEPROM.bin
endif

romfs.post::
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	sudo /sbin/ldconfig -r $(ROMFSDIR) -C /etc_ro/ld.so.cache
	rm -f $(RAMDISK)
	rm -f $(KERNELZ).lzma
ifeq ($(CONFIG_RT2880_ROOTFS_IN_FLASH),y)
	$(ROOTDIR)/tools/bin/mksquashfs_xz-4.0 $(ROMFSDIR) $(RAMDISK) -all-root -no-exports -noappend -nopad -noI -no-xattrs
endif

image:
	##############CREATE IMAGE#################
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S $(ROOTDIR)/$(LINUXDIR)/vmlinux $(KERNELZ)
	cd $(IMAGEDIR) ; rm -f $(KERNELZ).*; \
	$(ROOTDIR)/tools/bin/lzma_alone e -a1 -d25 $(KERNELZ) $(KERNELZ).$(COMP)
ifeq ($(CONFIG_BLK_DEV_INITRD),)
ifeq ($(CONFIG_ROOTFS_IN_FLASH_NO_PADDING),)
	@#============================================================
	@# uboot hdr(64B) + mtdblock0(192KB) + mtdblock1(64KB) + 
	@# mtdblock2(64KB) + mtdblock3 + mtdblock4 + mtdblock5
	@# 
	@# Padding size of mtdblock3 is equal to "MTD_KRN_PART_SIZE 
	@# - 64B - the size of compressed kernel image"
	@#
	@# In Other Words:
	@# MTD Kernel Partition Size is MUST bigger than 
	@# "Original Kernel Image Size + 64B"
	@#============================================================
	#========<< This is README not Warning Message>>==============
	# Please make sure that original kernel image size can't 
	# bigger or equal to Kernel Partition Size. 
	#
	# For Example:
	# 
	# Original kernel image size is 563421 Bytes(0x898DD Bytes).
	# Flash sector size is 0x10000, so MTD kernel 
	# partition size MUST bigger than 0x90000.
	#
	# #make menuconfig
	#
	# Kernel/Library/Defaults Selection  --->
	#   [*] Customize Kernel Settings (NEW)
	#      Machine selection  --->
	#        (90000) MTD Kernel Partition Size (Unit:Bytes)
	#=============================================================
	# Original Kernel Image Size
	@wc -c $(KERNELZ).$(COMP)
	# !!! Please make sure that Padded Kernel Image > Original Kernel Image Size !!!
ifeq ($(findstring 0x, $(CONFIG_MTD_KERNEL_PART_SIZ)),0x)
	@SIZE=`wc -c $(KERNELZ).$(COMP) | awk '{ print $$1 }'` ; \
	MTD_KRN_PART_SIZE=`printf "%d" $(CONFIG_MTD_KERNEL_PART_SIZ)` ; \
	PAD=`expr $$MTD_KRN_PART_SIZE - 64 - $$SIZE` ; \
	dd if=/dev/zero count=1 bs=$$PAD conv=sync 2> /dev/null | tr \\000 \\377 >> $(KERNELZ).$(COMP)
else
	@SIZE=`wc -c $(KERNELZ).$(COMP) | awk '{ print $$1 }'` ; \
	MTD_KRN_PART_SIZE=`printf "%d" 0x$(CONFIG_MTD_KERNEL_PART_SIZ)` ; \
	PAD=`expr $$MTD_KRN_PART_SIZE - 64 - $$SIZE` ; \
	dd if=/dev/zero count=1 bs=$$PAD conv=sync 2> /dev/null | tr \\000 \\377 >> $(KERNELZ).$(COMP)
endif
else
	@SIZE=`wc -c $(KERNELZ).$(COMP) | awk '{ print $$1 }'` ; \
	SIZE=`expr $$SIZE + 64` ; \
	DIV=`expr $$SIZE % $(PART_ALIGN)` ; \
	if [ $$DIV -gt 0 ] ; then \
		PAD=`expr $(PART_ALIGN) - $$DIV` ; \
		dd if=/dev/zero count=1 bs=$$PAD conv=sync 2> /dev/null | tr \\000 \\377 >> $(KERNELZ).$(COMP) ; \
	fi
endif
	# Padded Kernel Image Size
	@wc -c $(KERNELZ).$(COMP)
	# Original RootFs Size
	  @du -sb $(ROMFSDIR)
	# Compressed RootFs Size
	  @wc -c $(RAMDISK)
	# Padded Kernel Image + Compressed Rootfs Size
	  @cat $(RAMDISK) >> $(KERNELZ).$(COMP)
	  @wc -c $(KERNELZ).$(COMP)
	# !!! Please make sure that Padded Kernel Image + Compressed Rootfs size
ifeq ($(CONFIG_MTD_NAND_RALINK),y)
	# can't bigger than 33554432 (32MiB) !!!
else
ifneq ($(CONFIG_RT2880_FLASH_AUTO),y)
ifeq ($(CONFIG_RT2880_FLASH_32M),y)
	# can't bigger than 32964544 !!!
else
ifeq ($(CONFIG_RT2880_FLASH_16M),y)
	# can't bigger than 16187136 !!!
else
	# can't bigger than 7995136 !!!
endif
endif
else
	# can't bigger than: flash size - storage partition size -
	# uboot_hdr(64B) - mtd0(192KiB) - mtd1(64KiB) - mtd2(64KiB) !!!
endif
endif
	#===========================================
endif
	# Pack final image and write headers
ifeq ($(CONFIG_ROOTFS_IN_FLASH_NO_PADDING),)
	@cd $(IMAGEDIR) ; \
	ENTRY=`LANG=C LC_ALL=C readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}'` ; \
	$(MKIMAGE_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80000000 -e $$ENTRY \
	-n $(CONFIG_FIRMWARE_PRODUCT_ID) -V $(FIRMWARE_KERNEL_VER) $(FIRMWARE_ROOTFS_VER) -d $(KERNELZ).$(COMP) $(IMAGE)
else
	# For No padded, need write kernel size in image header 
	# to correct mount partition in mtd drivers address
	@cd $(IMAGEDIR) ; \
	ENTRY=`LANG=C LC_ALL=C readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}'` ; \
	ISIZE=`wc -c $(KERNELZ).$(COMP) | awk '{print $$1}'` ; \
	RSIZE=`wc -c $(RAMDISK) | awk '{print $$1}'` ; \
	KRN_SIZE=`expr $$ISIZE - $$RSIZE + 64` ; \
	$(MKIMAGE_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80000000 -e $$ENTRY -k $$KRN_SIZE \
	-n $(CONFIG_FIRMWARE_PRODUCT_ID) -V $(FIRMWARE_KERNEL_VER) $(FIRMWARE_ROOTFS_VER) -d $(KERNELZ).$(COMP) $(IMAGE)
endif
