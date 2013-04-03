obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o

ifeq ($(CONFIG_RALINK_RT3052),y)
hw_nat-objs += sys_rfrw.o
endif

hw_nat-objs += acl_policy.o acl_ioctl.o
hw_nat-objs += ac_policy.o ac_ioctl.o
hw_nat-objs += mtr_policy.o mtr_ioctl.o

### uncomment this flag if you want to send bcast/mcast traffic to PPE instead of CPU
#EXTRA_CFLAGS += -DHWNAT_MCAST_BCAST_PPE

### uncomment this flag if you want to offload all MBSS interfaces from WiFi
#EXTRA_CFLAGS += -DHWNAT_MBSS_ALL

### uncomment this flag if you want to set debug
#EXTRA_CFLAGS += -DHWNAT_DEBUG

