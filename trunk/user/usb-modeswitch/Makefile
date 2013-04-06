PROG        = usb_modeswitch
VERS        = 1.2.5
CFLAGS     += -Wall -ffunction-sections -fdata-sections
LDFLAGS    += -Wl,--gc-sections
LIBS        = -l usb
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX      = $(DESTDIR)/usr
ETCDIR      = $(DESTDIR)/etc
UDEVDIR     = $(DESTDIR)/lib/udev
SBINDIR     = $(PREFIX)/sbin

all: $(PROG)

$(PROG): $(OBJS) usb_modeswitch.h
	$(CC) -o $(PROG) $(OBJS) $(CFLAGS) $(LIBS) $(LDFLAGS)

clean:
	-rm -f $(PROG) *.o

romfs:
	$(ROMFSINST) /bin/$(PROG)
	cp -rf usb_modeswitch.d $(ROMFSDIR)/etc_ro
