CFLAGS += -fPIC
CFLAGS += -Wall -I. -I./include
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)
CFLAGS += $(if $(CONFIG_RT3352_INIC_MII),-DUSE_RT3352_MII,)
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)

LDFLAGS += -L.

OBJS := shutils.o netutils.o defaults.o nvram_linux.o notify_rc.o bin_sem_asus.o flash_mtd.o pids.o

all: libshared.so

libshared.so: $(OBJS)
	$(LD) -shared -o $@ $^

romfs:
	$(ROMFSINST) -S /lib/libshared.so

clean:
	rm -f *.o libshared.so
