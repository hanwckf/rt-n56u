include $(RT28xx_DIR)/os/linux/config.mk

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
MOD_NAME = rtutil$(CHIPSET)ap
DAT_PATH = /etc/Wireless/RT$(CHIPSET)AP
endif
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)
MOD_NAME = rtutil$(CHIPSET)sta
DAT_PATH = /etc/Wireless/RT$(CHIPSET)STA
endif
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
MOD_NAME = rtutil$(CHIPSET)apsta
endif
#endif // CONFIG_APSTA_SUPPORT //

obj-m := $(MOD_NAME).o

#ifdef CONFIG_AP_SUPPORT

rtutil$(CHIPSET)ap-objs := \
	../../os/linux/rt_linux.o

#ifdef RT2860
ifeq ($(CHIPSET),2860)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3090
ifeq ($(CHIPSET),3090)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT2870
ifeq ($(CHIPSET),2870)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT2070
ifeq ($(CHIPSET),2070)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3070
ifeq ($(CHIPSET),3070)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT2880
ifeq ($(CHIPSET),2880)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3572
ifeq ($(CHIPSET),3572)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3062
ifeq ($(CHIPSET),3062)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3562
ifeq ($(CHIPSET),3562)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3390
ifeq ($(CHIPSET),3390)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3370
ifeq ($(CHIPSET),3370)
rtutil$(CHIPSET)ap-objs += \
	../../os/linux/rt_usb_util.o
endif

#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_STA_SUPPORT

rtutil$(CHIPSET)sta-objs := \
	../../os/linux/rt_linux.o

#ifdef RT2860
ifeq ($(CHIPSET),2860)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3090
ifeq ($(CHIPSET),3090)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT2870
ifeq ($(CHIPSET),2870)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT2070
ifeq ($(CHIPSET),2070)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3070
ifeq ($(CHIPSET),3070)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT2880
ifeq ($(CHIPSET),2880)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3572
ifeq ($(CHIPSET),3572)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3062
ifeq ($(CHIPSET),3062)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3562
ifeq ($(CHIPSET),3562)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3390
ifeq ($(CHIPSET),3390)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3370
ifeq ($(CHIPSET),3370)
rtutil$(CHIPSET)sta-objs += \
	../../os/linux/rt_usb_util.o
endif

#endif // CONFIG_STA_SUPPORT //


#ifdef CONFIG_APSTA_SUPPORT
rtutil$(CHIPSET)apsta-objs := \
	../../os/linux/rt_linux.o

#ifdef RT2860
ifeq ($(CHIPSET),2860)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3090
ifeq ($(CHIPSET),3090)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT2870
ifeq ($(CHIPSET),2870)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT2070
ifeq ($(CHIPSET),2070)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3070
ifeq ($(CHIPSET),3070)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_usb_util.o
endif

#ifdef RT3390
ifeq ($(CHIPSET),3390)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_rbus_pci_util.o
endif

#ifdef RT3370
ifeq ($(CHIPSET),3370)
rtutil$(CHIPSET)apsta-objs += \
	../../os/linux/rt_usb_util.o
endif

#endif // CONFIG_APSTA_SUPPORT //

clean:
	rm -f ../../common/*.o
	rm -f ../../common/.*.{cmd,flags,d}
	rm -f ../../os/linux/*.{o,ko,mod.{o,c}}
	rm -f ../../os/linux/.*.{cmd,flags,d}
	rm -fr ../../os/linux/.tmp_versions
	rm -f ../../os/linux/Module.symvers
	rm -f ../../chips/*.o
	rm -f ../../chips/.*.{cmd,flags,d}
ifeq ($(RT28xx_MODE),AP)
	rm -f ../../ap/*.o
	rm -f ../../ap/.*.{cmd,flags,d}
else
ifeq ($(RT28xx_MODE),STA)
	rm -f ../../sta/*.o
	rm -f ../../sta/.*.{cmd,flags,d}
endif
endif
