EXEC = pppoe-discovery

RP_VERSION=3.12
CFLAGS += '-DRP_VERSION="$(RP_VERSION)"'
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)

all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ -fPIC $^

pppoe-discovery:  pppoe-discovery.o discovery.o if.o common.o debug.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) /usr/bin/pppoe-discovery

clean:
	rm -f $(EXEC) *.o

