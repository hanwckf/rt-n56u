TOP = $(ROOTDIR)/user

INSTALL	=	all
IFLAGS  =	-idirafter dummyinc

CFLAGS  +=      -DASUS_NVRAM
CFLAGS	+=	-Wall -W -Wshadow
CFLAGS  +=       -I$(TOP)/shared -I$(TOP)/shared/include
CFLAGS	+=      -I$(TOP)/libdisk

#LIBS	=	-ldl -lresolv -lcrypt

LINK	=	-Wl,-s
LDFLAGS	=	-L$(TOP)/libdisk -ldisk -L$(TOP)/shared -lshared

OBJS	=	main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
		tunables.o ftpdataio.o secbuf.o ls.o \
		postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
		banner.o filestr.o parseconf.o secutil.o \
		ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
		tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
		ssl.o sysutil.o sysdeputil.o

all:	$(OBJS)
	$(CC) -o vsftpd $(OBJS) $(LINK) $(LIBS) $(LDFLAGS)
#	$(STRIP) $@

clean:
	rm -f *.o *.swp vsftpd

.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

install: all
	if [ -f $(INSTALLDIR)/sbin/vsftpd ]; then rm -f $(INSTALLDIR)/sbin/vsftpd ; fi
	$(ROMFSINST) /sbin/vsftpd
	$(STRIP) $(INSTALLDIR)/sbin/vsftpd

romfs:
	$(STRIP) vsftpd
	$(ROMFSINST) /sbin/vsftpd

