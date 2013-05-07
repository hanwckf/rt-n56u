# api for old ASIC

### reduce code size (disable unused code)
API_8370_REDUCE_CODE=y

EXTRA_CFLAGS += -D_LITTLE_ENDIAN

ifeq ($(API_8370_REDUCE_CODE),y)
EXTRA_CFLAGS += -D_REDUCE_CODE
endif

ifeq ($(CONFIG_RTL8367_CIF_MDIO),y)
EXTRA_CFLAGS += -DMDC_MDIO_OPERATION
endif

obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv.o

ifneq ($(API_8370_REDUCE_CODE),y)
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_acl.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_eee.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_eeelldp.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_fc.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_inbwctrl.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_mirror.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_oam.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_qos.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_rldp.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_rrcp.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_scheduling.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_specialCongest.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_svlan.o
endif

obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_cputag.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_dot1x.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_green.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_igmp.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_interrupt.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_led.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_lut.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_meter.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_mib.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_misc.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_phy.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_port.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_portIsolation.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_rma.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_storm.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_trunking.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_unknownMulticast.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtl8370_asicdrv_vlan.o
obj-$(CONFIG_RTL8367_API_8370)		+= rtk_api.o

