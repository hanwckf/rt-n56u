obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o

ifneq ($(CONFIG_HNAT_V2),y)
hw_nat-objs += acl_policy.o acl_ioctl.o
hw_nat-objs += ac_policy.o ac_ioctl.o
hw_nat-objs += mtr_policy.o mtr_ioctl.o
endif


