OBJS = usb_info.o disk_io_tools.o disk_initial.o disk_share.o
#OBJS += disk_swap.o

TOP = $(ROOTDIR)/user

CFLAGS += -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/shared/include
LDFLAGS = -L$(TOP)/shared -lshared

all: libdisk.so test_share

libdisk.so: $(OBJS) $(TOP)/shared/shutils.o
	$(LD) -shared -o $@ $^

test_share: test_share.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

#disk_test: disk_test.o libdisk.so
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

romfs:
	$(ROMFSINST) /lib/libdisk.so
	if [ "$(CONFIG_FIRMWARE_INCLUDE_SMBD)" = "y" ] || [ "$(CONFIG_FIRMWARE_INCLUDE_FTPD)" = "y" ] ; then \
		$(ROMFSINST) /sbin/test_share ; \
	fi;

clean:
	rm -f *.o libdisk.so test_share
