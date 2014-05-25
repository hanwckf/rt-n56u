USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared

CFLAGS += -Wall -I. -I$(SHDIR) -I$(SHDIR)/include -I$(USERDIR)/libdisk -I$(USERDIR)/wireless_tools

include ../shared/boards.mk
include ../shared/cflags.mk

LDFLAGS  = -L$(SHDIR) -L$(USERDIR)/libdisk -L$(USERDIR)/wireless_tools
LDFLAGS += -lm -lshared -ldisk -liw
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
LDFLAGS += -L$(STAGEDIR)/lib -lssl -lcrypto
endif

EXEC = httpd
OBJS = httpd.o ej.o cgi.o web_ex.o common.o nvram_x.o ralink.o crc32.o initial_web_hook.o aspbw.o
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
OBJS += https.o
endif
ifdef CONFIG_RTL8367
OBJS += switch_rtl8367.o
else
ifdef CONFIG_RAETH_ESW
OBJS += switch_mtk_esw.o
endif
endif

all: $(OBJS) Makefile
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

c.o:
	$(CC) -c $*.c $(CFLAGS)

clean:
	rm -f *.o *~ httpd

romfs:
	$(ROMFSINST) /usr/sbin/httpd
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
	$(ROMFSINST) /usr/bin/https-cert.sh
endif
