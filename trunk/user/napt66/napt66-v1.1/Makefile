PWD  := $(shell pwd)
KDIR := ../../../linux-3.4.x

obj-m := napt66.o
napt66-objs := napt66_main.o napt66_conntrack.o napt66_nat.o napt66_hash_table.o napt66_ftp_alg.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules ARCH=mips CROSS_COMPILE=../../toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
