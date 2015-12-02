###############################################################################
#  Jan 2007 Bruce Chang
#
#  Initial Release
#
#
#
###############################################################################

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

obj-y := reset.o init.o memory.o printf.o cmdline.o setup.o time.o

ifdef CONFIG_IRQ_GIC
obj-y += irq-gic.o
else
obj-y += irq.o
endif

obj-$(CONFIG_MIPS_CMP) += malta-amon.o

obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_MTD_NAND_MTK) += dev-nand.o
obj-$(CONFIG_MTK_XHCI) += dev-xhci.o uphy.o
obj-$(CONFIG_RT3XXX_EHCI_OHCI) += dev-ehci_ohci.o uphy.o
obj-$(CONFIG_USB_GADGET_RT) += dev-udc.o
obj-$(CONFIG_RALINK_DMA) += dev-dma.o
obj-$(CONFIG_RALINK_TIMER_DFS) += rt_timer_dfs.o
obj-$(CONFIG_RALINK_TIMER_WDG) += rt_timer_wdg.o
obj-$(CONFIG_DWC_OTG) += lm.o

ifdef CONFIG_MTK_MMC
obj-y += dev-mmc.o
endif

ifdef CONFIG_I2C_RALINK
obj-y += dev-i2c.o
endif
