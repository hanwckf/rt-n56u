# api for new ASIC

### reduce code size (disable unused code)
API_8367B_REDUCE_CODE=y

EXTRA_CFLAGS += -D_LITTLE_ENDIAN

ifeq ($(API_8367B_REDUCE_CODE),y)
EXTRA_CFLAGS += -D_REDUCE_CODE
endif

ifeq ($(CONFIG_RTL8367_CIF_MDIO),y)
EXTRA_CFLAGS += -DMDC_MDIO_OPERATION
endif

ifeq ($(CONFIG_RTL8367_ASIC_RB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367RB
endif

ifeq ($(CONFIG_RTL8367_ASIC_RVB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367R_VB
endif

ifeq ($(CONFIG_RTL8367_ASIC_MB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367MB
endif

ifeq ($(CONFIG_RTL8367_ASIC_MVB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367M_VB
endif

obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv.o
ifneq ($(API_8367B_REDUCE_CODE),y)
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_eav.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_eee.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_eeelldp.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_fc.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_inbwctrl.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_mirror.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_oam.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_qos.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_rldp.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_rrcp.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_specialCongest.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_igmp.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_rma.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_scheduling.o
endif
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_acl.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_cputag.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_dot1x.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_green.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_interrupt.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_hsb.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_led.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_lut.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_meter.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_mib.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_misc.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_phy.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_port.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_portIsolation.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_storm.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_trunking.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_unknownMulticast.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_vlan.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtl8367b_asicdrv_svlan.o
obj-$(CONFIG_RTL8367_API_8367B)		+= rtk_api.o

