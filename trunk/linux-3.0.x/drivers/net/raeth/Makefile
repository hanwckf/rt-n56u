obj-$(CONFIG_RAETH) += raeth.o

raeth-objs := raether.o mii_mgr.o ra_mac.o ra_phy.o ra_ioctl.o

#EXTRA_CFLAGS += -DRAETH_DEBUG

ifeq ($(CONFIG_RALINK_RT3052),y)
ifeq ($(CONFIG_RAETH_ESW),y)
raeth-objs += ra_rfrw.o
endif
endif

ifeq ($(CONFIG_RAETH_ESW_CONTROL),y)
raeth-objs += ra_esw_ioctl.o
endif

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RALINK_RT3052_MP2),y)
raeth-objs += mcast.o
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_RAETH_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif


