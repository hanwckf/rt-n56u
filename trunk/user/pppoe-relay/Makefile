# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux user-space PPPoE client.
#
# Copyright (C) 2000 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# LIC: GPL
#
# $Id: Makefile,v 1.1.1.1 2007/02/15 12:14:31 jiahao Exp $
#***********************************************************************

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.10


DEFINES=
# Plugin for pppd on Linux
LINUX_KERNELMODE_PLUGIN=
PPPD_INCDIR=

# PPPoE relay -- currently only supported on Linux
PPPOE_RELAY=pppoe-relay

# Include ServPoET version if we're building for ServPoET

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS   = -s -O2
CFLAGS  += -I. -I$(SHDIR)/include -I$(LINUXINCDIR)
CFLAGS  += '-DVERSION="$(VERSION)"'
TARGET   = pppoe-relay

all: $(TARGET)
	#$(STRIP) $(TARGET)
	#$(INSTALL) $(TARGET) /root/broadcom/target/usr/sbin/

pppoe-relay: relay.o debug.o if.o common.o
	$(CC) -o $@ $^ $(CFLAGS) -o pppoe-relay

#install: all
#	install -D pppoe-relay $(INSTALLDIR)/usr/sbin/pppoe-relay
#	$(STRIP) $(INSTALLDIR)/usr/sbin/pppoe-relay

romfs: all
	$(STRIP) pppoe-relay
	$(ROMFSINST) /usr/sbin/pppoe-relay

clean:
	rm -f *.o pppoe-relay
