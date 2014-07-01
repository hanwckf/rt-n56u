EXEC     = hub-ctrl
OBJS     = hub-ctrl.c
CFLAGS  += -I$(STAGEDIR)/include/libusb-1.0
CFLAGS  += -Wall -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections
LIBS     = -lusb-1.0

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

romfs:
	$(ROMFSINST) /usr/bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.o
