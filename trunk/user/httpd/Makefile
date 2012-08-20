-include $(LINUX_CONFIG)

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

DOIPV6 = $(if $(CONFIG_IPV6),-DUSE_IPV6,)

CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include -I$(USERDIR)/libdisk -I$(USERDIR)/iptables/include -I$(USERDIR)/wireless_tools
CFLAGS += -I$(LINUXINCDIR)
CFLAGS += -Wall -DTRANSLATE_ON_FLY
CFLAGS += $(DOIPV6)

LDFLAGS	= -L$(SHDIR) -L$(USERDIR)/libdisk -L$(USERDIR)/iptables/libiptc -L$(USERDIR)/wireless_tools
LDFLAGS += -lm -lshared -ldisk -liptc -liw

#CFLAGS += -DW7_LOGO
#CFLAGS += -DWIFI_LOGO

EXEC = httpd
OBJS = httpd.o ej.o cgi.o web_ex.o common.o nvram_f.o broadcom.o crc32.o initial_web_hook.o aspbw.o netconf_linux.o

all:	$(OBJS) Makefile
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

c.o:
	$(CC) -c $*.c $(CFLAGS)

clean:
	rm -f *.o *~ httpd

romfs:
	$(ROMFSINST) /usr/sbin/httpd
