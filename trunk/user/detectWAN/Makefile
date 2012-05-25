
-include $(LINUX_CONFIG)

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

PHYMAC = $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)

CFLAGS  += -Wall -DASUS_NVRAM -I. -I$(SHDIR)/include -I$(SHDIR)
CFLAGS  += $(PHYMAC)

LDFLAGS += -L$(SHDIR) -lshared

all: detectWan

detectWan: detectWAN.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

romfs:
	$(ROMFSINST) /sbin/detectWan

clean:
	rm -f detectWan *~ *.o *.so 
