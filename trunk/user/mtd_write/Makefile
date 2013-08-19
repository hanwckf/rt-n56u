EXEC = mtd_write
OBJS = mtd.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
CFLAGS  += -ffunction-sections -fdata-sections

LDFLAGS += -static
LDFLAGS += -Wl,--gc-sections

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	rm -f $(EXEC) *.o

