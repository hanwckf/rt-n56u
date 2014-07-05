#
#   Makefile - build and install the comgt package
#   Copyright (C) 2005  Martin Gregorie
#   Copyright (C) 2006  Paul Hardwick
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    martin@gregorie.org, paul@peck.org.uk
#
#    $Id: Makefile,v 1.2 2009-02-09 06:58:39 michael Exp $
#
#

EXEC     = comgt
OBJS     = comgt.c
CFLAGS  := $(CFLAGS) -ffunction-sections -fdata-sections
LDFLAGS := $(LDFLAGS) -Wl,--gc-sections

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) scripts/devices /etc_ro/ppp/3g
	$(ROMFSINST) scripts/command /etc_ro/ppp/3g/command
	$(ROMFSINST) scripts/dump /etc_ro/ppp/3g/dump
	$(ROMFSINST) scripts/operator /etc_ro/ppp/3g/operator
	$(ROMFSINST) scripts/getinfo /etc_ro/ppp/3g/getinfo
	$(ROMFSINST) scripts/read_sms /etc_ro/ppp/3g/read_sms
	$(ROMFSINST) scripts/send_sms /etc_ro/ppp/3g/send_sms

clean:
	-rm -f $(EXEC) *.o *~
