EXEC = qdma
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

ifdef CONFIG_RALINK_MT7621
CFLAGS += -DCONFIG_RALINK_MT7621
endif

SRC = qdma.c

all: $(EXEC)

qdma: $(SRC) $(CONF_H)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/qdma

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

