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
	DEBFLAGS = -s
endif

CFLAGS += -DSUPPORT_LPRng
CFLAGS += -I$(TOP)/shared
CFLAGS += -Wall -I.
CFLAGS += $(DEBFLAGS)
CFLAGS += -ffunction-sections -fdata-sections

LDFLAGS += -lpthread
LDFLAGS += -L$(TOP)/shared -lshared
LDFLAGS += -lusb
LDFLAGS += -Wl,--gc-sections

all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) -o $(PROGS) $(OBJS) $(CFLAGS) $(LDFLAGS)

romfs:
	$(ROMFSINST) /usr/sbin/$(PROGS)

clean:
	rm -f $(PROGS) *.o *~
