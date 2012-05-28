obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o

ifeq ($(CONFIG_RALINK_RT3052),y)
hw_nat-objs += sys_rfrw.o
endif

ifneq ($(CONFIG_HNAT_V2),y)
hw_nat-objs += acl_policy.o acl_ioctl.o
hw_nat-objs += ac_policy.o ac_ioctl.o
hw_nat-objs += mtr_policy.o mtr_ioctl.o
endif

#uncomment this flag if you want to send bcast/mcast from external if and wifi to cpu instead PPE
EXTRA_CFLAGS += -DHWNAT_SKIP_MCAST_BCAST

#uncomment this flag if you want to send 8021q from external if and wifi to cpu instead PPE
EXTRA_CFLAGS += -DHWNAT_RA_SKIP_8021Q

#uncomment this flag if you want to set debug
#EXTRA_CFLAGS += -DHWNAT_DEBUG
