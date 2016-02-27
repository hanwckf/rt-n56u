
API_RTL8367B=n
CFLAGS	+= -D_LITTLE_ENDIAN

ifeq ($(SWITCH_CTRLIF_MDIO),y)
CFLAGS	+= -DMDC_MDIO_OPERATION
endif

ifeq ($(SWITCH_ASIC_RTL8367RB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8367RB
endif

ifeq ($(SWITCH_ASIC_RTL8367RVB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8367R_VB
endif

ifeq ($(SWITCH_ASIC_RTL8367MB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8367MB
endif

ifeq ($(SWITCH_ASIC_RTL8367MVB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8367M_VB
endif

ifeq ($(SWITCH_ASIC_RTL8365MB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8365MB
endif

ifeq ($(SWITCH_ASIC_RTL8368MB),y)
API_RTL8367B=y
CFLAGS	+= -DCHIP_RTL8368MB
endif

OBJS	+= rtl8367/ralink_smi.o rtl8367/rtl8367.o

ifeq ($(API_RTL8367B),y)
CFLAGS	+= -DAPI_RTL8367B
OBJS	+= rtl8367/api_8367b/rtk_api.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv_hsb.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv_led.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv_phy.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv_port.o
OBJS	+= rtl8367/api_8367b/rtl8367b_asicdrv_portIsolation.o
else
OBJS	+= rtl8367/api_8370/rtk_api.o
OBJS	+= rtl8367/api_8370/rtl8370_asicdrv.o
OBJS	+= rtl8367/api_8370/rtl8370_asicdrv_led.o
OBJS	+= rtl8367/api_8370/rtl8370_asicdrv_phy.o
OBJS	+= rtl8367/api_8370/rtl8370_asicdrv_port.o
OBJS	+= rtl8367/api_8370/rtl8370_asicdrv_portIsolation.o
endif

