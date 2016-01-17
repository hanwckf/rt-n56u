ifndef ROOTDIR
ROOTDIR=../..
endif

USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared

include $(SHDIR)/boards.mk
include $(SHDIR)/cflags.mk

CFLAGS += -Wall -I. -I$(SHDIR) -I$(SHDIR)/include
CFLAGS += -I$(USERDIR)/wireless_tools

LDFLAGS += -L. -lm
LDFLAGS += -L$(SHDIR) -lshared
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
ifeq ($(STORAGE_ENABLED),y)
CFLAGS  += -I$(USERDIR)/libdisk
LDFLAGS += -L$(USERDIR)/libdisk -ldisk
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
LDFLAGS += -L$(STAGEDIR)/lib -lssl -lcrypto
endif

EXEC = httpd
OBJS = httpd.o ej.o cgi.o web_ex.o common.o nvram_x.o ralink.o crc32.o base64.o aspbw.o initial_web_hook.o variables.o
OBJS += tdate_parse.o upload.o
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
OBJS += https.o
endif
ifeq ($(STORAGE_ENABLED),y)
OBJS += aidisk.o
endif
ifdef CONFIG_RTL8367
OBJS += switch_rtl8367.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/rtl8367
else
ifdef CONFIG_MT7530_GSW
OBJS += switch_mtk_esw.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth
else
ifdef CONFIG_RAETH_ESW
OBJS += switch_mtk_esw.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth
endif
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
