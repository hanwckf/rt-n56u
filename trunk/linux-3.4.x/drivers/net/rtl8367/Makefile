#
# Makefile for the rtl8367 driver
#

### reduce API code size (disable unused code)
API_REDUCE_CODE=y

EXTRA_CFLAGS += -D_LITTLE_ENDIAN

ifeq ($(API_REDUCE_CODE),y)
EXTRA_CFLAGS += -D_REDUCE_CODE
endif

EXTRA_CFLAGS += -DDISABLE_VLAN_SHADOW

ifeq ($(CONFIG_RTL8367_CIF_MDIO),y)
EXTRA_CFLAGS += -DMDC_MDIO_OPERATION
endif

# api for old ASIC (RTL8367R, RTL8367M, RTL8370M)
ifeq ($(CONFIG_RTL8367_API_8370),y)

ifneq ($(API_REDUCE_CODE),y)
rtl8367-y	+= api_8370/rtl8370_asicdrv_acl.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_eeelldp.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_fc.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_inbwctrl.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_mirror.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_oam.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_qos.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_rldp.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_rrcp.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_scheduling.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_specialCongest.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_svlan.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_cputag.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_igmp.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_rma.o
endif
rtl8367-y	+= api_8370/rtl8370_asicdrv_dot1x.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_green.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_eee.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_interrupt.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_led.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_lut.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_meter.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_mib.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_misc.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_phy.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_port.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_portIsolation.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_storm.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_trunking.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_unknownMulticast.o
rtl8367-y	+= api_8370/rtl8370_asicdrv_vlan.o
rtl8367-y	+= api_8370/rtl8370_asicdrv.o
rtl8367-y	+= api_8370/rtk_api.o
endif

# api for new ASIC
ifeq ($(CONFIG_RTL8367_API_8367B),y)

ifeq ($(CONFIG_RTL8367_ASIC_RB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367RB
endif
ifeq ($(CONFIG_RTL8367_ASIC_RVB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367R_VB
endif
ifeq ($(CONFIG_RTL8367_ASIC_MVB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367M_VB
endif
ifeq ($(CONFIG_RTL8367_ASIC_MB),y)
EXTRA_CFLAGS += -DCHIP_RTL8367MB
endif
ifeq ($(CONFIG_RTL8365_ASIC_MB),y)
EXTRA_CFLAGS += -DCHIP_RTL8365MB
endif
ifeq ($(CONFIG_RTL8368_ASIC_MB),y)
EXTRA_CFLAGS += -DCHIP_RTL8368MB
endif

ifneq ($(API_REDUCE_CODE),y)
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_eav.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_fc.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_inbwctrl.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_mirror.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_qos.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_igmp.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_rma.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_scheduling.o
endif
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_acl.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_cputag.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_dot1x.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_green.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_eee.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_interrupt.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_hsb.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_led.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_lut.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_meter.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_mib.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_misc.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_phy.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_port.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_portIsolation.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_storm.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_trunking.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_unknownMulticast.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_vlan.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv_svlan.o
rtl8367-y	+= api_8367b/rtl8367b_asicdrv.o
rtl8367-y	+= api_8367b/rtk_api.o
endif

# common driver
rtl8367-$(CONFIG_RTL8367_CIF_SMI)	+= ralink_smi.o
rtl8367-$(CONFIG_RTL8367_CIF_MDIO)	+= ralink_mdio.o
rtl8367-$(CONFIG_RTL8367_IGMP_SNOOPING)	+= rtl8367_igmp.o
rtl8367-y				+= rtl8367_drv.o

# MT7621/MT7623 MCM (MT7530 GSW) stuff
rtl8367-$(CONFIG_RTL8367_MCM_WAN_PORT)	+= mt7530_mcm.o

obj-$(CONFIG_RTL8367)			+= rtl8367.o
