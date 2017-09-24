.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean

ifndef ROOTDIR
ROOTDIR=..
endif
ifndef ROMFSDIR
ROMFSDIR=$(ROOTDIR)/romfs
endif

UCLINUX_BUILD_LIB=1
-include $(LINUX_CONFIG)
include $(PROJECT_CONFIG)
include $(ARCH_CONFIG)

CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

DESTDIR=$(ROOTDIR)/stage

dir_y =
dir_n =
dir_  =

LIBS_INCLUDE_LIBLZO=n
LIBS_INCLUDE_LIBSSL=n
LIBS_INCLUDE_LIBICONV=n
LIBS_INCLUDE_LIBUSB_0=n
LIBS_INCLUDE_LIBUSB_1=n
LIBS_INCLUDE_MEDIA=n
STORAGE_ENABLED=n

ifdef CONFIG_MMC_BLOCK
STORAGE_ENABLED=y
endif
ifdef CONFIG_BLK_DEV_SD
STORAGE_ENABLED=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_EAP_PEAP),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_DDNS_SSL),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SFTP),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENVPN),y)
LIBS_INCLUDE_LIBLZO=y
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSH),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SSWAN),y)
LIBS_INCLUDE_LIBSSL=y
endif

ifeq ($(STORAGE_ENABLED),y)
ifeq ($(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA),y)
LIBS_INCLUDE_LIBSSL=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_MINIDLNA),y)
LIBS_INCLUDE_LIBICONV=y
LIBS_INCLUDE_MEDIA=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y)
LIBS_BUILD_SHARED=y
LIBS_INCLUDE_LIBICONV=y
LIBS_INCLUDE_MEDIA=y
endif
endif

ifdef CONFIG_USB_SUPPORT
LIBS_INCLUDE_LIBUSB_1=y
ifeq ($(CONFIG_FIRMWARE_INCLUDE_U2EC),y)
LIBS_INCLUDE_LIBUSB_0=y
endif
endif

ifdef CONFIG_MTD_UBI
LIBS_INCLUDE_LIBLZO=y
endif

dir_y						+= libz
dir_$(LIBS_INCLUDE_LIBLZO)			+= liblzo
dir_$(LIBS_INCLUDE_LIBICONV)			+= libiconv
dir_$(LIBS_INCLUDE_LIBSSL)			+= libssl
dir_$(LIBS_INCLUDE_LIBUSB_0)			+= libusb
dir_$(LIBS_INCLUDE_LIBUSB_1)			+= libusb-1.0
dir_y						+= libnfnetlink

# ipset shared libs
dir_$(CONFIG_FIRMWARE_INCLUDE_IPSET)		+= libmnl

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
	for i in `ls -d *` ; do \
		if [ -d $$i ] ; then \
			$(MAKE) -C $$i clean ; \
			rm -f $(DESTDIR)/.installed_$$i ; \
		fi ; \
	done

