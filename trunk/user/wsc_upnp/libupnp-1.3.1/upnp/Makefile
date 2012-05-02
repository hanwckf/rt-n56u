TOPDIR=../../

CFLAGS =-DHAVE_CONFIG_H -I. -I. -I.. -I../upnp/inc -I./inc \
	-I../threadutil/inc -I../ixml/inc -I./src/inc

LIBUPNP=libupnp.a
LIBUPNP_VERS=1.3.1
LIBUPNP_SHARED=libupnp.so.$(LIBUPNP_VERS)

OBJS    := src/api/upnpapi.o src/api/upnptools.o
OBJS    += src/gena/gena_callback2.o  src/gena/gena_ctrlpt.o  src/gena/gena_device.o
OBJS    += src/uuid/md5.o  src/uuid/sysdep.o  src/uuid/uuid.o
OBJS    += src/urlconfig/urlconfig.o
OBJS    += src/soap/soap_common.o  src/soap/soap_ctrlpt.o  src/soap/soap_device.o
OBJS    += src/ssdp/ssdp_ctrlpt.o  src/ssdp/ssdp_device.o  src/ssdp/ssdp_server.o
OBJS    += src/genlib/client_table/client_table.o \
	   src/genlib/service_table/service_table.o \
	   src/genlib/miniserver/miniserver.o \
	   src/genlib/net/sock.o \
	   src/genlib/net/http/httpreadwrite.o \
	   src/genlib/net/http/webserver.o \
           src/genlib/net/http/parsetools.o \
           src/genlib/net/http/httpparser.o \
	   src/genlib/net/http/statcodes.o \
	   src/genlib/net/uri/uri.o \
	   src/genlib/util/util.o \
	   src/genlib/util/strintmap.o \
	   src/genlib/util/membuffer.o \
	   src/genlib/util/upnp_timeout.o \


all: $(LIBUPNP)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBUPNP_SHARED) \
		-o $(LIBUPNP_SHARED) --whole-archive $(LIBUPNP) \
		-lc -lpthread ../ixml/libixml.so.$(LIBUPNP_VERS) ../threadutil/libthreadutil.so.$(LIBUPNP_VERS)  $(LDADD_LIBFLOAT);

$(LIBUPNP): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBUPNP) $(OBJS)

$(OBJS): Makefile

clean:
	find ./ -name "*.o" -exec rm -f {} \;

