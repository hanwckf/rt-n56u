obj-$(CONFIG_MTD_RALINK)        += ralink-flash.o
obj-$(CONFIG_MTD_NAND_RALINK)	+= rt_nand.o
obj-$(CONFIG_MTD_SPI_RALINK)	+= rt_spi.o 

rt_nand-objs = ralink_nand.o gdma.o
rt_spi-objs = ralink_spi.o

# this is decrease code size.
# flash chips is slow, support code no need fast speed
EXTRA_CFLAGS += -Os -fno-caller-saves
