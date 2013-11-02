obj-$(CONFIG_MTD_RALINK)	+= ralink-flash.o
obj-$(CONFIG_MTD_NAND_RALINK)	+= rt_nand.o
obj-$(CONFIG_MTD_SPI_RALINK)	+= rt_spi.o

ifneq ($(CONFIG_RALINK_RT2880),y)
rt_spi-objs = ralink_spi.o

ifeq ($(CONFIG_RALINK_RT3883),y)
rt_nand-objs = ralink_nand_rt3052.o gdma.o
else
ifeq ($(CONFIG_RALINK_RT3052),y)
rt_nand-objs = ralink_nand_rt3052.o gdma.o
else
rt_nand-objs = ralink_nand.o
endif
endif

endif
