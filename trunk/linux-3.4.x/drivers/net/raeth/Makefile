obj-$(CONFIG_RAETH) += raeth.o

raeth-objs := raether.o mii_mgr.o ra_mac.o ra_phy.o ra_ioctl.o

ifeq ($(CONFIG_RALINK_RT3052),y)
raeth-objs += ra_esw_rt305x.o
ifeq ($(CONFIG_RAETH_ESW),y)
raeth-objs += ra_rfrw.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
raeth-objs += ra_esw_rt305x.o
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
raeth-objs += ra_esw_rt305x.o
endif

ifeq ($(CONFIG_RALINK_MT7628),y)
raeth-objs += ra_esw_rt305x.o
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
raeth-objs += ra_esw_mt7620.o
ifeq ($(CONFIG_MT7530_GSW),y)
raeth-objs += ra_gsw_mt7530.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
ifeq ($(CONFIG_MT7530_GSW),y)
raeth-objs += ra_esw_mt7621.o
endif
endif

ifeq ($(CONFIG_RAETH_ESW),y)
raeth-objs += ra_esw_base.o
endif

ifeq ($(CONFIG_MT7530_GSW),y)
raeth-objs += ra_esw_base.o
endif

ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += ra_esw_ioctl.o
else
ifeq ($(CONFIG_RAETH_DHCP_TOUCH),y)
raeth-objs += ra_esw_dhcpc.o
endif
endif

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_RAETH_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif
