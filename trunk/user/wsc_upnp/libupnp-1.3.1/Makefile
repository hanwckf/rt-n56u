LIBUPNP=libupnp.a
LIBIXML=libixml.a
LIBTHREADUTIL=libthreadutil.a
CFLAGS += -Ithreadutil/inc  -Iixml/inc -Iupnp/inc

all: $(LIBUPNP) $(LIBIXML) $(LIBTHREADUTIL)

shared:
	$(MAKE) -C ixml shared
	$(MAKE) -C threadutil shared
	$(MAKE) -C upnp shared

$(LIBUPNP): 
	$(MAKE) -C upnp

$(LIBIXML): 
	$(MAKE) -C ixml

$(LIBTHREADUTIL): 
	$(MAKE) -C threadutil

$(OBJS): Makefile

clean:
	$(MAKE) -C ixml clean
	$(MAKE) -C threadutil clean
	$(MAKE) -C upnp clean

