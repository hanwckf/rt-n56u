obj-$(CONFIG_RAETH) += raeth.o

raeth-objs := raether.o ra_mac.o mii_mgr.o

ifeq ($(CONFIG_RT_3052_ESW),y)
raeth-objs += ra_rfrw.o
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

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

