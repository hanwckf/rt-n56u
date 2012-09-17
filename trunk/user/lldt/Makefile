VPATH = src
CFLAGS += -Os -pipe -Wall -Wno-unused
OS_LAYER = osl-linux.c

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user
CFLAGS  += -I. -I$(SHDIR) -I$(SHDIR)/include
CFLAGS  += $(if $(CONFIG_IPV6),-DUSE_IPV6,)

LDFLAGS += -L$(SHDIR) -lshared

all: lld2d

include src/common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend

clear:
	rm -f *.o lld2d

-include .depend
