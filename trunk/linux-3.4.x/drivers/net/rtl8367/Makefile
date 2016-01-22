#
# Makefile for the rtl8367
#

EXTRA_CFLAGS += -D_LITTLE_ENDIAN

# api for old ASIC
obj-$(CONFIG_RTL8367_API_8370)		+= api_8370/

# api for new ASIC
obj-$(CONFIG_RTL8367_API_8367B)		+= api_8367b/

# common driver
obj-$(CONFIG_RTL8367_CIF_SMI)		+= ralink_smi.o
obj-$(CONFIG_RTL8367_CIF_MDIO)		+= ralink_mdio.o
obj-$(CONFIG_RTL8367_IGMP_SNOOPING)	+= rtl8367_igmp.o
obj-$(CONFIG_RTL8367)			+= rtl8367_drv.o

