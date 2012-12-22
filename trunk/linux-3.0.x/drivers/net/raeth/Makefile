obj-$(CONFIG_RAETH) += raeth.o

ifeq ($(CONFIG_RAETH_ASUS_N65U_OBJ),y)
raeth-objs := raeth_asus.obj
else
raeth-objs := raether.o ra_mac.o mii_mgr.o

#EXTRA_CFLAGS   += -DCONFIG_RAETH_MULTIPLE_RX_RING

ifeq ($(CONFIG_RT_3052_ESW),y)
raeth-objs += ra_rfrw.o
endif

ifeq ($(CONFIG_RAETH_QOS),y)
raeth-objs += ra_qos.o
endif

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RALINK_RT3052_MP2),y)
raeth-objs += mcast.o
endif

ifeq ($(CONFIG_RAETH_NETLINK),y)
raeth-objs += ra_netlink.o
endif
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

ifeq ($(CONFIG_TASKLET_WORKQUEUE_SW),y)
EXTRA_CFLAGS += -DTASKLET_WORKQUEUE_SW
endif
