#
# U2EC Makefile
#
# Copyright (C) 2008 ASUSTeK Corporation
#
#

TOP := $(shell pwd)/..
SRCBASE := $(TOP)/..
INSTALLDIR := $(ROOTDIR)/romfs

PROGS	= u2ec
OBJS	= usbsock.o testusb.o
DEBUG	= n

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DU2EC_DEBUG
	DEBFLAGS += -DPDEBUG_SENDSECV
	DEBFLAGS += -DPDEBUG_DECODE
	OBJS += debug.o decode.o
else
	DEBFLAGS = -s -O2
endif

CFLAGS += -DSUPPORT_LPRng
CFLAGS += -I$(TOP)/shared -I$(TOP)/shared/include
CFLAGS += -Wall -I.
CFLAGS += $(DEBFLAGS)

LDFLAGS += -lpthread
LDFLAGS += -lusb
LDFLAGS += -L$(TOP)/shared -lshared


all: $(PROGS)
	$(STRIP) $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

#ifneq ($(strip $(CROSS_COMPILE)),)
install: $(PROGS)
	install -D $(PROGS) $(INSTALLDIR)/usr/sbin/$(PROGS)
#	$(STRIP) $(INSTALLDIR)/usr/sbin/$(PROGS)
romfs:
	$(ROMFSINST) /usr/sbin/$(PROGS)
#endif

clean:
	rm -f $(PROGS) *.o *~
