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

obj-y := reset.o init.o irq.o memory.o printf.o cmdline.o setup.o time.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_RALINK_TIMER_DFS) += rt_timer_dfs.o
obj-$(CONFIG_RALINK_TIMER_WDG) += rt_timer_wdg.o
obj-$(CONFIG_RALINK_DMA) +=  dev-dma.o
obj-$(CONFIG_DWC_OTG) += lm.o
obj-$(CONFIG_RT3XXX_EHCI_OHCI) +=  ehci_ohci.o
obj-$(CONFIG_USB_GADGET_RT) += udc.o
