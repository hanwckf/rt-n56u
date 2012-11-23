OBJS = usb_info.o disk_io_tools.o disk_initial.o disk_share.o disk_swap.o

TOP = $(ROOTDIR)/user

CFLAGS += -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/shared/include
LDFLAGS = -L$(TOP)/shared -lshared

all: libdisk.so test_share

libdisk.so: $(OBJS) $(TOP)/shared/shutils.o
	$(LD) -shared -o $@ $^

test_share: test_share.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

romfs: all
	$(ROMFSINST) /lib/libdisk.so
	$(ROMFSINST) /sbin/test_share

clean:
	rm -f *.o libdisk.so test_share
