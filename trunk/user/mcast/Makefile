EXEC = mcast speed

CFLAGS += -O3
#CFLAGS += -g
LDFLAGS =

#CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)

all: $(EXEC)

	@# @echo "-$(CFLAGS)-$(LDFLAGS)-"
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

mcast: mcast.o ifaddr.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

speed: speed.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
romfs:
	$(ROMFSINST) mcast /usr/bin/mcast
	$(ROMFSINST) speed /usr/bin/speed
clean:
	rm -f $(EXEC) *.o
