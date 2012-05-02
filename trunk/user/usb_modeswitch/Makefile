PROG        = usb_modeswitch
VERS        = 1.2.2
CFLAGS     += -Wall
LIBS        = -lusb
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX      = 
ETCDIR      = /etc
UDEVDIR     = /lib/udev
SBINDIR     = /sbin
MANDIR      = /share/man/man1

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(CFLAGS) $(LIBS) $(LDFLAGS)

romfs:
	$(ROMFSINST) /bin/$(PROG)

clean:
	-rm -f $(PROG) *.o

