SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include -I$(USERDIR)/libdisk -I$(USERDIR)/wireless_tools
CFLAGS += -Wall -DTRANSLATE_ON_FLY
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)
CFLAGS += $(if $(CONFIG_FIRMWARE_NVRAM_NOCOMMIT),-DNVRAM_NOCOMMIT,)

LDFLAGS  = -L$(SHDIR) -L$(USERDIR)/libdisk -L$(USERDIR)/wireless_tools
LDFLAGS += -lm -lshared -ldisk -liw
ifeq ($(KERNEL3X),y)
CFLAGS += -DUSE_KERNEL3X
endif

#CFLAGS += -DW7_LOGO
#CFLAGS += -DWIFI_LOGO

EXEC = httpd
OBJS = httpd.o ej.o cgi.o web_ex.o common.o nvram_x.o ralink.o crc32.o initial_web_hook.o aspbw.o

all:	$(OBJS) Makefile
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

c.o:
	$(CC) -c $*.c $(CFLAGS)

clean:
	rm -f *.o *~ httpd

romfs:
	$(ROMFSINST) /usr/sbin/httpd
