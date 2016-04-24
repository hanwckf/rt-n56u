# $Id: Makefile,v 1.8 2016/02/09 09:36:30 nanard Exp $
CFLAGS?=-Wall -g -D_GNU_SOURCE -DDEBUG -Wstrict-prototypes -Wdeclaration-after-statement
CC?=gcc

# the following sucks, but works
LIBS  = $(IPTABLESPATH)/libiptc/.libs/libip4tc.a
ifeq ($(ENABLE_IPV6),1)
LIBS += $(IPTABLESPATH)/libiptc/.libs/libip6tc.a
endif
CFLAGS += -DIPTABLES_143 -I$(IPTABLESPATH)/include/
LDFLAGS += -L$(IPTABLESPATH)/libiptc/.libs

all:	iptcrdr.o iptpinhole.o
#        testiptcrdr_peer testiptcrdr_dscp test_nfct_get
#        testiptcrdr testiptpinhole

clean:
	$(RM) *.o testiptcrdr testiptpinhole testiptcrdr_peer test_nfct_get \
        testiptcrdr_dscp

testiptcrdr:	testiptcrdr.o upnpglobalvars.o $(LIBS)

testiptcrdr_peer:	testiptcrdr_peer.o upnpglobalvars.o $(LIBS)

testiptcrdr_dscp:	testiptcrdr_dscp.o upnpglobalvars.o $(LIBS)

testiptpinhole:	testiptpinhole.o iptpinhole.o upnpglobalvars.o $(LIBS)

test_nfct_get:	test_nfct_get.o test_nfct_get.o -lmnl -lnetfilter_conntrack

test_nfct_get.o:	test_nfct_get.c

testiptcrdr_peer.o:	testiptcrdr_peer.c

testiptcrdr_dscp.o:	testiptcrdr_dscp.c

iptcrdr.o:	iptcrdr.c iptcrdr.h

iptpinhole.o:	iptpinhole.c iptpinhole.h

upnpglobalvars.o:	../upnpglobalvars.c ../upnpglobalvars.h
	$(CC) -c -o $@ $<


#depends
testiptcrdr.o: testiptcrdr.c iptcrdr.c
testiptcrdr_dscp.o:	testiptcrdr_dscp.c iptcrdr.c
testiptcrdr_peer.o:	testiptcrdr_peer.c iptcrdr.c
test_nfct_get.o:	test_nfct_get.c nfct_get.c
