ifndef ROOTDIR
ROOTDIR=../..
endif

CFLAGS  += -ffunction-sections -fdata-sections -fvisibility=hidden
LDFLAGS += -Wl,--gc-sections

SHDIR		= $(ROOTDIR)/user/shared
CFLAGS		+= -I$(SHDIR)
LDFLAGS		+= -L$(SHDIR) -lshared

all:
	$(CC) $(CFLAGS) $(LDFLAGS) wsdd2.c wsd.c llmnr.c -o wsdd2

install:

clean:
	rm -f wsdd2

romfs:
	$(ROMFSINST) wsdd2 /sbin/wsdd2

