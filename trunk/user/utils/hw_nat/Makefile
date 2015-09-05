EXEC = hw_nat
ifndef CONFIG_HNAT_V2
EXEC += acl ac mtr
endif

HWNAT_OBJS = hw_nat.o hwnat_api.o util.o
AC_OBJS = ac.o ac_api.o util.o
ACL_OBJS = acl.o acl_api.o util.o
MTR_OBJS = mtr.o mtr_api.o util.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/net/nat/hw_nat

ifdef CONFIG_HNAT_V2
CFLAGS += -DCONFIG_HNAT_V2
ifdef CONFIG_RALINK_MT7621
CFLAGS += -DCONFIG_RALINK_MT7621
ifdef CONFIG_RA_HW_NAT_MCAST
CFLAGS += -DCONFIG_PPE_MCAST
endif
endif
endif

all: $(EXEC)

hw_nat: $(HWNAT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(HWNAT_OBJS)

acl: $(ACL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(ACL_OBJS)

ac: $(AC_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(AC_OBJS)

mtr: $(MTR_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(MTR_OBJS)

romfs:
	$(ROMFSINST) /bin/hw_nat
#ifndef CONFIG_HNAT_V2
#	$(ROMFSINST) /bin/acl
#	$(ROMFSINST) /bin/ac
#	$(ROMFSINST) /bin/mtr
#endif

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


