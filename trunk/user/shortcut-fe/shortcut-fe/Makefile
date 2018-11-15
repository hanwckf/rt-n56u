PWD  := $(shell pwd)
KDIR := $(ROOTDIR)/$(CONFIG_LINUXDIR)

ccflags-y += -DSFE_SUPPORT_IPV6

obj-m := shortcut-fe.o shortcut-fe-ipv6.o fast-classifier.o

shortcut-fe-objs := sfe_ipv4.o

shortcut-fe-ipv6-objs := sfe_ipv6.o

all:
	$(MAKEARCH_KERNEL) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
