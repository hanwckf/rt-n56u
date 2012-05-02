obj-$(CONFIG_RA_HW_NAT) += hw_nat.o

hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o

ifeq ($(CONFIG_RA_HW_NAT_MINIMAL),)
hw_nat-objs += acl_policy.o acl_ioctl.o
hw_nat-objs += ac_policy.o ac_ioctl.o
hw_nat-objs += mtr_policy.o mtr_ioctl.o
endif

#uncomment this flag if you want to send bcast/mcast to cpu instead PPE
EXTRA_CFLAGS += -DHWNAT_SPKIP_MCAST_BCAST

#uncomment this flag if you need fix gre process
EXTRA_CFLAGS += -DHWNAT_FIX_GRE

#uncomment this flag if you want to set user autobind mode
#EXTRA_CFLAGS += -DHWNAT_USER_AUTO

#uncomment this flag if you want to set debug
#EXTRA_CFLAGS += -DHWNAT_DEBUG

#uncomment this flag if you want to set debug
#EXTRA_CFLAGS += -DHWNAT_DEBUG_PKT_PARSE
