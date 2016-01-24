EXEC = switch mii_mgr mii_mgr_cl45
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

ifdef CONFIG_RALINK_MT7620
SRC = switch_gsw.c
CFLAGS += -DCONFIG_RALINK_MT7620
endif

ifdef CONFIG_RALINK_MT7621
SRC = switch_gsw.c
CFLAGS += -DCONFIG_RALINK_MT7621
endif

ifdef CONFIG_RALINK_MT7628
SRC = switch.c
CFLAGS += -DCONFIG_RALINK_MT7628
endif

ifdef CONFIG_MT7530_GSW
SRC = switch_gsw.c
CFLAGS += -DCONFIG_MT7530_GSW
endif

all: $(EXEC)

switch: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mii_mgr: mii_mgr.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mii_mgr_cl45: mii_mgr_cl45.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/switch
	$(ROMFSINST) /sbin/mii_mgr
	$(ROMFSINST) /sbin/mii_mgr_cl45

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

