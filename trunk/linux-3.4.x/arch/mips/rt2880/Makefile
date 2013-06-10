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

obj-y    := reset.o init.o irq.o \
            memory.o printf.o cmdline.o setup.o time.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_RALINK_TIMER_DFS) +=  rt_timer.o

obj-$(CONFIG_RT3XXX_OHCI) +=  ehci_ohci.o
obj-$(CONFIG_RALINK_RT3352) +=  ehci_ohci.o
obj-$(CONFIG_RALINK_RT5350) +=  ehci_ohci.o
obj-$(CONFIG_EARLY_PRINTK)  +=  early_printk.o
