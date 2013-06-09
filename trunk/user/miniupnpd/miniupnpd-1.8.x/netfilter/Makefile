# $Id: Makefile,v 1.6 2012/04/26 13:50:48 nanard Exp $
CFLAGS?=-Wall -g -D_GNU_SOURCE -DDEBUG -Wstrict-prototypes -Wdeclaration-after-statement -ansi
CC?=gcc

LIBS = -lip4tc

CFLAGS := $(CFLAGS) -DIPTABLES_143 -I$(IPTABLESPATH)/include/
LDFLAGS := $(LDFLAFGS) -L$(IPTABLESPATH)/libiptc/

# the following sucks, but works
LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.o

all:	iptcrdr.o testiptcrdr iptpinhole.o testiptpinhole

clean:
	$(RM) *.o testiptcrdr testiptpinhole

testiptcrdr:	testiptcrdr.o iptcrdr.o upnpglobalvars.o $(LIBS)

testiptpinhole:	testiptpinhole.o iptpinhole.o upnpglobalvars.o $(LIBS)

iptcrdr.o:	iptcrdr.c iptcrdr.h

iptpinhole.o:	iptpinhole.c iptpinhole.h

upnpglobalvars.o:	../upnpglobalvars.c ../upnpglobalvars.h
	$(CC) -c -o $@ $<


