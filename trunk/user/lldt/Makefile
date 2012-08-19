VPATH = src
CFLAGS += -O2 -pipe -Wall -Wno-unused -s -fpack-struct -Wpadded
LD= $(CC)
OS_LAYER = osl-linux.c

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user
CFLAGS  += -Wall -I. -I$(SHDIR)/include 
LDFLAGS += -L$(SHDIR) -lshared

all: lld2d lld2test

include src/common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend

clear:
	rm -f *.o lld2d lld2test

-include .depend
