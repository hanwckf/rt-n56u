EXEC = qdma
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

SRC = qdma.c

all: $(EXEC)

qdma: $(SRC) $(CONF_H)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/qdma

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

