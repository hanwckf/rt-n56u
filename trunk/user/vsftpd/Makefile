# Makefile for systems with GNU tools

TOP = $(ROOTDIR)/user

INSTALL	= install
IFLAGS  = -idirafter dummyinc

CFLAGS	+= -Wall -W -Wshadow
CFLAGS  += -I$(TOP)/shared -I$(TOP)/shared/include
CFLAGS	+= -I$(TOP)/libdisk
CFLAGS	+= -ffunction-sections -fdata-sections

ifneq ($(CONFIG_RT2880_FLASH_16M),y)
CFLAGS += -Os
else
CFLAGS += -O2
endif

LIBS	= #-lcrypt
LINK	= -Wl,-s

LDFLAGS	= -L$(TOP)/libdisk -ldisk -L$(TOP)/shared -lshared
LDFLAGS	+= -Wl,--gc-sections

OBJS	= main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
	tunables.o ftpdataio.o secbuf.o ls.o asus_ext.o \
	postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
	banner.o filestr.o parseconf.o secutil.o \
	ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
	tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
	ssl.o sslslave.o ptracesandbox.o ftppolicy.o sysutil.o sysdeputil.o

.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

vsftpd: $(OBJS) 
	$(CC) -o vsftpd $(OBJS) $(LINK) $(LIBS) $(LDFLAGS)

clean:
	rm -f *.o *.swp vsftpd

romfs:
	$(STRIP) vsftpd
	$(ROMFSINST) /sbin/vsftpd

