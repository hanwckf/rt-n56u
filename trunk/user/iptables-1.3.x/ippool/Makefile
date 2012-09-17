EXTRAS+=$(shell [ -f $(KERNEL_DIR)/include/linux/netfilter_ipv4/ip_pool.h ] && echo ippool/libippool.a ippool/ippool)

ifndef TOPLEVEL_INCLUDED
local:
	cd .. && $(MAKE) $(KERN_TARGETS) $(SHARED_LIBS) $(EXTRAS)

else
EXTRA_DEPENDS+=$(shell [ -f $(KERNEL_DIR)/include/linux/netfilter_ipv4/ip_pool.h ] && echo ippool/libippool.d)

ippool/libippool.a: ippool/libippool.a(ippool/libippool.o)

ippool/libippool.d: %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | sed -e 's@^.*\.o:@$*.d $*.a($*.o):@' > $@

ippool/ippool.o: ippool/ippool.c
ippool/ippool: ippool/ippool.o ippool/libippool.a
	$(CC) $(CFLAGS) -o $@ $^
endif
