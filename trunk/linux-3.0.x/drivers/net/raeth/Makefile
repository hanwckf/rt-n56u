obj-$(CONFIG_RAETH) += raeth.o

raeth-objs := raether.o mii_mgr.o ra_ioctl.o ra_mac.o ra_phy.o ra_eth.o

ifeq ($(CONFIG_RALINK_RT3052),y)
raeth-objs += ra_rfrw.o
raeth-objs += ra_esw_rt305x.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_rt305x.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
raeth-objs += ra_esw_rt305x.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_rt305x.o
endif
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
raeth-objs += ra_esw_rt305x.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_rt305x.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
raeth-objs += ra_esw_mt7620.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_mt762x.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
raeth-objs += ra_eth_mt7621.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_mt762x.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7628),y)
raeth-objs += ra_esw_rt305x.o
ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += mtk_esw/ioctl_rt305x.o
endif
endif

ifeq ($(CONFIG_RAETH_ESW_IGMP_SNOOP_SW),y)
raeth-objs += mtk_esw/ioctl_igmp.o
endif

ifeq ($(CONFIG_RAETH_ESW),y)
raeth-objs += ra_esw_base.o
endif

ifeq ($(CONFIG_MT7530_GSW),y)
raeth-objs += ra_esw_base.o
raeth-objs += ra_gsw_mt7530.o
endif

ifeq ($(CONFIG_RAETH_DHCP_TOUCH),y)
ifneq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += ra_esw_dhcpc.o
endif
endif

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RAETH_BOTH_GMAC),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_RAETH_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif
