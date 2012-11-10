SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include -I$(USERDIR)/libdisk -I$(USERDIR)/wireless_tools
CFLAGS += -Wall -DTRANSLATE_ON_FLY
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)
CFLAGS += $(if $(CONFIG_RT3352_INIC_MII),-DUSE_RT3352_MII,)
CFLAGS += $(if $(CONFIG_RTL8367_API_8367B),-DUSE_RTL8367_API_8367B,)
CFLAGS += $(if $(CONFIG_FIRMWARE_NVRAM_NOCOMMIT),-DNVRAM_NOCOMMIT,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_MINIDLNA),-DAPP_MINIDLNA,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_FIREFLY),-DAPP_FIREFLY,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION),-DAPP_TRMD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_ARIA),-DAPP_ARIA,)

LDFLAGS  = -L$(SHDIR) -L$(USERDIR)/libdisk -L$(USERDIR)/wireless_tools
LDFLAGS += -lm -lshared -ldisk -liw

ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N56U")
CFLAGS += -DBOARD_N56U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N65U")
CFLAGS += -DBOARD_N65U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"SWR1100")
CFLAGS += -DBOARD_SWR1100
endif

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
