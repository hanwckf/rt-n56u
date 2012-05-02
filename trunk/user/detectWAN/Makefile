SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS  += -Wall -DASUS_NVRAM -I. -I$(SHDIR)/include -I$(SHDIR)
LDFLAGS += -L$(SHDIR) -lshared

all: detectWan

detectWan: detectWAN.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

romfs:
	$(ROMFSINST) /sbin/detectWan

clean:
	rm -f detectWan *~ *.o *.so 
