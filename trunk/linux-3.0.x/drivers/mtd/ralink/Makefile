obj-$(CONFIG_MTD_RALINK)        += ralink-flash.o
obj-$(CONFIG_MTD_NAND_RALINK)	+= rt_nand.o
obj-$(CONFIG_MTD_SPI_RALINK)	+= rt_spi.o

ifneq ($(CONFIG_RALINK_RT2880),y)
rt_spi-objs = ralink_spi.o

ifneq ($(CONFIG_RALINK_RT3352),y)
ifneq ($(CONFIG_RALINK_RT5350),y)
rt_nand-objs = ralink_nand.o gdma.o
endif
endif

endif
