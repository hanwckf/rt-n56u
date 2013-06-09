INSTALL=install
INSTALLDIR=install -m 0755 -d
INSTALLMAN=install -m 0644

SUBDIRS = man3 man8

all:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir; done

distclean: clean

clean:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

install:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir install; done

.PHONY: install clean distclean

.EXPORT_ALL_VARIABLES:
