PWD  := $(shell pwd)
KDIR := $(ROOTDIR)/$(CONFIG_LINUXDIR)

obj-m := napt66.o
napt66-objs := napt66_main.o napt66_conntrack.o napt66_nat.o napt66_hash_table.o napt66_ftp_alg.o

all:
	$(MAKEARCH_KERNEL) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
