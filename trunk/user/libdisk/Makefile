USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared

CFLAGS += -fPIC
CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include

LDFLAGS = -L$(SHDIR) -lshared

OBJS = usb_info.o disk_io_tools.o disk_initial.o disk_share.o
#OBJS += disk_swap.o

all: libdisk.so test_share

libdisk.so: $(OBJS) $(SHDIR)/shutils.o
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
