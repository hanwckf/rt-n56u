.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean

ifndef ROOTDIR
ROOTDIR=..
endif
ifndef ROMFSDIR
ROMFSDIR=$(ROOTDIR)/romfs
endif

include $(PROJECT_CONFIG)
include $(ARCH_CONFIG)

DESTDIR=$(ROOTDIR)/stage

LIBC_DIR = uClibc-0.9.33.2

all:
	$(MAKE) -j$(HOST_NCPU) -C $(LIBC_DIR) && \
	$(MAKE) -C $(LIBC_DIR) utils || \
	exit $$? ; \
	if [ ! -f $(DESTDIR)/.installed_$(LIBC_DIR) ] ; then \
		$(MAKE) -C $(LIBC_DIR) install_runtime && \
		$(MAKE) -C $(LIBC_DIR) install_utils && \
		touch $(DESTDIR)/.installed_$(LIBC_DIR) ; \
	fi

%_only:
	$(MAKE) -C $(@:_only=)

%_romfs:
	$(MAKE) -C $(@:_romfs=) romfs

%_clean:
	$(MAKE) -C $(@:_clean=) clean

romfs:
	$(STRIP) $(DESTDIR)/bin/ldd ; \
	$(STRIP) $(DESTDIR)/sbin/ldconfig ; \
	cp -fP $(DESTDIR)/bin/ldd $(ROMFSDIR)/bin ; \
	cp -fP $(DESTDIR)/lib/libcrypt.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libcrypt-*.so $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libdl*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libm*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libnsl*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libpthread*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libresolv*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/librt*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libutil*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libuClibc*.so* $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/libc.so.0 $(ROMFSDIR)/lib ; \
	cp -fP $(DESTDIR)/lib/ld-uClibc*.so* $(ROMFSDIR)/lib ; \
	chmod +x $(ROMFSDIR)/lib/libuClibc*.so ; \
	chmod +x $(ROMFSDIR)/lib/libpthread*.so ; \
	chmod +x $(ROMFSDIR)/lib/libm*.so ; \
	chmod +x $(ROMFSDIR)/lib/libcrypt*.so ; \
	chmod +x $(ROMFSDIR)/lib/libdl*.so ; \
	chmod +x $(ROMFSDIR)/lib/libresolv*.so ; \
	chmod +x $(ROMFSDIR)/lib/libutil*.so ; \
	chmod +x $(ROMFSDIR)/lib/librt*.so ; \
	chmod +x $(ROMFSDIR)/lib/libnsl*.so

clean:
	rm -f $(DESTDIR)/.installed_$(LIBC_DIR)
	$(MAKE) -C $(LIBC_DIR) distclean
