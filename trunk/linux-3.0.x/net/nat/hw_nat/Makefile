obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o

ifneq ($(CONFIG_HNAT_V2),y)
hw_nat-objs += acl_policy.o acl_ioctl.o
hw_nat-objs += ac_policy.o ac_ioctl.o
hw_nat-objs += mtr_policy.o mtr_ioctl.o
endif

ifeq ($(CONFIG_RA_HW_NAT_MCAST),y)
ifeq ($(CONFIG_RALINK_MT7621),y)
EXTRA_CFLAGS += -DCONFIG_PPE_MCAST
hw_nat-objs += mcast_tbl.o
endif
endif

