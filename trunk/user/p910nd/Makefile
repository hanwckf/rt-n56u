# If you don't have it in /var/log/subsys, uncomment and define
#CFLAGS += -DLOCKFILE_DIR=\"/var/log\"

PROG = p910nd
CONFIG = p910nd.conf
INITSCRIPT = p910nd.init
MANPAGE = p910nd.8
INSTALL = install
BINDIR = /usr/sbin
CONFIGDIR = /etc/sysconfig
SCRIPTDIR = /etc/init.d
MANDIR = /usr/share/man/man8

$(PROG): p910nd.c
	$(CC) -o $@ $^ $(CFLAGS) $(DEFINES) $(LIBS)

strip: $(PROG)
	$(STRIP) -s $(PROG)

clean:
	rm -f *.o $(PROG)

romfs:
	$(ROMFSINST) /usr/sbin/p910nd
