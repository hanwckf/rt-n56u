EXEC = switch mii_mgr
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

ifdef CONFIG_RALINK_MT7620
CFLAGS += -DCONFIG_RALINK_MT7620
else
ifdef CONFIG_RALINK_MT7621
CFLAGS += -DCONFIG_RALINK_MT7621
else
ifdef CONFIG_RALINK_MT7628
CFLAGS += -DCONFIG_RALINK_MT7628
endif
endif
endif

ifdef CONFIG_MT7530_GSW
CFLAGS += -DCONFIG_MT7530_GSW
endif

all: $(EXEC)

switch: switch_gsw.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mii_mgr: mii_mgr.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/switch
	$(ROMFSINST) /sbin/mii_mgr

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

