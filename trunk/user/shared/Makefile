CFLAGS += -fPIC
CFLAGS += -Wall -I. -I./include
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)

LDFLAGS += -L.

OBJS := shutils.o http.o defaults.o nvram_linux.o notify_rc.o bin_sem_asus.o 
OBJS += semaphore.o flash_mtd.o pids.o files.o

all: libshared.so

libshared.so: $(OBJS)
	$(LD) -shared -o $@ $^

romfs:
	$(ROMFSINST) -S /lib/libshared.so

clean:
	rm -f *.o libshared.so
