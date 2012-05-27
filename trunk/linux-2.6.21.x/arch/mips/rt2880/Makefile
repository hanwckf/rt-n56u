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

obj-y    			:= mipsIRQ.o reset.o init.o irq.o memory.o cmdline.o setup.o time.o
obj-$(CONFIG_PCI) 		+= pci.o
obj-$(CONFIG_DWC_OTG) 		+= lm.o
obj-$(CONFIG_RALINK_TIMER_WDG)	+= ralink_wdt.o
obj-$(CONFIG_RALINK_TIMER_DFS)	+= rt_timer.o

obj-$(CONFIG_KGDB) 		+= gdb_hook.o

ifdef CONFIG_USB
obj-$(CONFIG_RALINK_RT3883)	+= ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_RT3352)	+= ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_RT5350)	+= ehci_ohci.o udc.o
endif

obj-$(CONFIG_KERNEL_NVRAM)	+= nvram.o
