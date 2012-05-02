EXEC = mtd_write
OBJS = mtd.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	rm -f $(EXEC) *.o

