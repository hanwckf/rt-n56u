OBJS = usb_info.o disk_io_tools.o disk_initial.o disk_share.o disk_swap.o

TOP = $(ROOTDIR)/user

CFLAGS += -fPIC
CFLAGS += -DASUS_NVRAM
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/shared/include
LDFLAGS = -L$(TOP)/shared -lshared

all: libdisk.so

libdisk.so: $(OBJS) $(TOP)/shared/shutils.o
	$(LD) -shared -o $@ $^

romfs: all
	$(ROMFSINST) -S /lib/libdisk.so

clean:
	rm -f *.o libdisk.so
