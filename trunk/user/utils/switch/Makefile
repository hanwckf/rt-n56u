EXEC = switch_gsw
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

ifdef CONFIG_RALINK_MT7620
CFLAGS += -DCONFIG_RALINK_MT7620
else
ifdef CONFIG_RALINK_MT7621
CFLAGS += -DCONFIG_RALINK_MT7621
endif
endif

ifdef CONFIG_MT7530_GSW
CFLAGS += -DCONFIG_MT7530_GSW
endif

SRC = switch_gsw.c

all: $(EXEC)
$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

