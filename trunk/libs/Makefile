.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean

ifndef ROOTDIR
ROOTDIR=..
endif
ifndef ROMFSDIR
ROMFSDIR=$(ROOTDIR)/romfs
endif

UCLINUX_BUILD_LIB=1
-include $(CONFIG_CONFIG)
-include $(LINUX_CONFIG)
-include $(ARCH_CONFIG)

CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

DESTDIR=$(ROOTDIR)/stage

dir_y =
dir_n =
dir_  =

# build shared libs for minidlna and firefly
ifeq ($(CONFIG_FIRMWARE_INCLUDE_MINIDLNA),y)
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y)
LIBS_BUILD_SHARED=y
endif
endif

LIBS_INCLUDE_LIBSSL=$(if $(CONFIG_FIRMWARE_INCLUDE_SFTP)$(CONFIG_FIRMWARE_INCLUDE_OPENSSH)$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)$(CONFIG_FIRMWARE_INCLUDE_ARIA),y,n)
LIBS_INCLUDE_LIBICONV=$(if $(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)$(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y,n)
LIBS_INCLUDE_MEDIA=$(if $(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)$(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y,n)

dir_y						+= libz
dir_$(LIBS_INCLUDE_LIBICONV)			+= libiconv
dir_$(LIBS_INCLUDE_LIBSSL)			+= libssl
dir_y						+= libusb

# tcpdump shared libs
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)		+= libpcap

# minidlna and firefly shared libs
dir_$(LIBS_INCLUDE_MEDIA)			+= libid3tag
dir_$(LIBS_INCLUDE_MEDIA)			+= libav
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)		+= libexif
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)		+= libjpeg
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)		+= libogg
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)		+= libvorbis
dir_$(LIBS_INCLUDE_MEDIA)			+= libflac
dir_$(LIBS_INCLUDE_MEDIA)			+= libsqlite

# transmission shared libs
dir_$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)	+= libcurl
dir_$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)	+= libevent

# aria2 shared libs
dir_$(CONFIG_FIRMWARE_INCLUDE_ARIA)		+= libxml2

all:
	for i in $(dir_y) ; do \
		[ -d $$i ] && \
		$(MAKE) -j1 -C $$i || \
		exit $$? ; \
		if [ ! -f $(DESTDIR)/.installed_$$i ] ; then \
			$(MAKE) -C $$i install && \
			touch $(DESTDIR)/.installed_$$i ; \
		fi ; \
	done

%_only:
	$(MAKE) -C $(@:_only=)

%_romfs:
	$(MAKE) -C $(@:_romfs=) romfs

%_clean:
	$(MAKE) -C $(@:_clean=) clean

romfs:
	for i in $(dir_y) ; do \
		[ -d $$i ] && \
		$(MAKE) -C $$i romfs ; \
	done

clean:
	for i in $(dir_y) $(dir_n) $(dir_) ; do \
		if [ -d $$i ] ; then \
			$(MAKE) -C $$i clean ; \
			rm -f $(DESTDIR)/.installed_$$i ; \
		fi ; \
	done

