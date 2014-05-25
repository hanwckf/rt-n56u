CFLAGS += -fPIC
CFLAGS += -Wall -I. -I./include

include boards.mk
include cflags.mk

LDFLAGS += -L.

OBJS := shutils.o netutils.o defaults.o nvram_linux.o notify_rc.o bin_sem_asus.o flash_mtd.o pids.o

all: libshared.so

libshared.so: $(OBJS)
	$(LD) -shared -o $@ $^

romfs:
	$(ROMFSINST) -S /lib/libshared.so

clean:
	rm -f *.o libshared.so
