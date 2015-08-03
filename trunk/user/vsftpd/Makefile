# Makefile for systems with GNU tools

TOP	 = $(ROOTDIR)/user

IFLAGS	 = -idirafter dummyinc

CFLAGS	+= -O2 -Wall -W -Werror -Wshadow -Wformat-security -D_FORTIFY_SOURCE=2
CFLAGS	+= -I$(TOP)/shared
CFLAGS	+= -I$(TOP)/libdisk
CFLAGS	+= -ffunction-sections -fdata-sections

LIBS	= #-lcrypt
LINK	= -Wl,-s

LDFLAGS	 = -L$(TOP)/libdisk -ldisk -L$(TOP)/shared -lshared
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FTPD_SSL),y)
CFLAGS	+= -DVSF_BUILD_SSL
LDFLAGS	+= -L$(STAGEDIR)/lib -lssl -lcrypto
endif
LDFLAGS	+= -Wl,--gc-sections

OBJS	= main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
	tunables.o ftpdataio.o secbuf.o ls.o asus_ext.o \
	postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
	banner.o filestr.o parseconf.o secutil.o \
	ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
	tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
	ssl.o sslslave.o ptracesandbox.o ftppolicy.o sysutil.o sysdeputil.o \
	seccompsandbox.o

.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

vsftpd: $(OBJS) 
	$(CC) -o vsftpd $(OBJS) $(LINK) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o *.swp vsftpd

romfs:
	mkdir -p $(INSTALLDIR)/usr/share/empty
	$(ROMFSINST) /sbin/vsftpd
