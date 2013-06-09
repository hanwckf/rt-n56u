ROOTDIR=$(DESTDIR)
PREFIX=/usr
LIBDIR=$(PREFIX)/lib
SBINDIR=/sbin
CONFDIR=/etc/iproute2
DATADIR=$(PREFIX)/share
DOCDIR=$(DATADIR)/doc/iproute2
MANDIR=$(DATADIR)/man
ARPDDIR=/var/lib/arpd

# Path to db_185.h include
#DBM_INCLUDE:=$(ROOTDIR)/usr/include

SHARED_LIBS = n

DEFINES= -DRESOLVE_HOSTNAMES -DLIBDIR=\"$(LIBDIR)\"
ifneq ($(SHARED_LIBS),y)
DEFINES+= -DNO_SHARED_LIBS
endif

DEFINES+=-DCONFDIR=\"$(CONFDIR)\"

#options if you have a bind>=4.9.4 libresolv (or, maybe, glibc)
#LDLIBS=-lresolv
LDLIBS=
ADDLIB=

#options for decnet
ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
ADDLIB+=ipx_ntop.o ipx_pton.o

HOSTCC = gcc
CFLAGS += -Wstrict-prototypes
CFLAGS += -I../include
CFLAGS += -D_GNU_SOURCE $(DEFINES)
CFLAGS += -ffunction-sections -fdata-sections
YACCFLAGS = -d -t -v

SUBDIRS=lib tc
ifdef CONFIG_IPV6
	SUBDIRS += ip
endif

LIBNETLINK=../lib/libnetlink.a ../lib/libutil.a
LDLIBS += $(LIBNETLINK)
LDFLAGS += -Wl,--gc-sections

all:
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i; done

install: all
	install -m 0755 -d $(DESTDIR)$(SBINDIR)
	install -m 0755 -d $(DESTDIR)$(CONFDIR)
	install -m 0755 -d $(DESTDIR)$(ARPDDIR)
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples/diffserv
	install -m 0644 README.iproute2+tc $(shell find examples -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples
	install -m 0644 $(shell find examples/diffserv -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples/diffserv
	@for i in $(SUBDIRS) doc; do $(MAKE) -C $$i install; done
	install -m 0644 $(shell find etc/iproute2 -maxdepth 1 -type f) $(DESTDIR)$(CONFDIR)

snapshot:
	echo "static const char SNAPSHOT[] = \""`date +%y%m%d`"\";" \
		> include/SNAPSHOT.h

clean:
	@for i in $(SUBDIRS) doc; \
	do $(MAKE) $(MFLAGS) -C $$i clean; done

clobber:
	$(MAKE) $(MFLAGS) clean
	rm -f cscope.*

distclean: clobber

cscope:
	cscope -b -q -R -Iinclude -sip -slib -smisc -snetem -stc

.EXPORT_ALL_VARIABLES:
