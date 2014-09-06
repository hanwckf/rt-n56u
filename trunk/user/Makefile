.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean

ifndef ROOTDIR
ROOTDIR=..
endif
ifndef ROMFSDIR
ROMFSDIR=$(ROOTDIR)/romfs
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
include $(PROJECT_CONFIG)
include $(ARCH_CONFIG)

INSTALLDIR = $(ROOTDIR)/romfs

USB_ENABLED=n
FS_EXT_ENABLED=n
FS_FAT_ENABLED=n
FS_NTFS_ENABLED=n
OPENSSH_ENABLED=n
DROPBEAR_ENABLED=n
ifneq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSH),y)
ifeq ($(CONFIG_FIRMWARE_INCLUDE_DROPBEAR),y)
DROPBEAR_ENABLED=y
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SFTP),y)
OPENSSH_ENABLED=y
endif
endif
else
OPENSSH_ENABLED=y
endif

ifdef CONFIG_USB_SUPPORT
USB_ENABLED=y
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT2),y)
FS_EXT_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT3),y)
FS_EXT_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXT4),y)
FS_EXT_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_FAT),y)
FS_FAT_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_ENABLE_EXFAT),y)
FS_FAT_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NTFS_3G),y)
FS_NTFS_ENABLED=y
endif
endif

dir_y =
dir_n =
dir_  =

dir_y							+= busybox
dir_y							+= iptables
dir_$(CONFIG_FIRMWARE_INCLUDE_IPSET)			+= ipset
dir_y							+= ebtables
dir_y							+= iproute2
dir_y							+= wireless_tools
dir_y							+= shared
dir_y							+= libdisk
dir_y							+= nvram
dir_y							+= mtd_write
dir_y							+= networkmap
dir_y							+= httpd
dir_y							+= www
dir_y							+= rc
dir_y							+= infosvr
dir_$(CONFIG_FIRMWARE_INCLUDE_FTPD)			+= vsftpd
dir_$(CONFIG_FIRMWARE_INCLUDE_SMBD)			+= samba3
dir_y							+= inadyn
dir_y							+= miniupnpd
dir_y							+= dnsmasq
dir_$(CONFIG_IPV6)					+= radvd
dir_y							+= igmpproxy
dir_y							+= udpxy
dir_y							+= nfsd
dir_$(USB_ENABLED)					+= util-linux
dir_$(CONFIG_FIRMWARE_INCLUDE_HDPARM)			+= hdparm
dir_y							+= utils
dir_y							+= lanauth
dir_y							+= 802.1x
dir_y							+= wpa_supplicant
dir_y							+= pppd
dir_y							+= accel-pptpd
dir_y							+= xl2tpd
dir_y							+= rp-l2tp
dir_y							+= lldt
dir_$(FS_EXT_ENABLED)					+= e2fsprogs
dir_$(FS_FAT_ENABLED)					+= dosfstools
dir_$(FS_NTFS_ENABLED)					+= ntfs-3g
dir_$(CONFIG_FIRMWARE_INCLUDE_LPRD)			+= LPRng
dir_$(CONFIG_FIRMWARE_INCLUDE_U2EC)			+= u2ec
dir_$(USB_ENABLED)					+= p910nd
dir_$(USB_ENABLED)					+= usb-modeswitch
dir_$(USB_ENABLED)					+= comgt-0.32
dir_$(USB_ENABLED)					+= uqmi
dir_y							+= optware
dir_y							+= scripts

# tcpdump
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)			+= tcpdump

# parted
dir_$(CONFIG_FIRMWARE_INCLUDE_PARTED)			+= parted

# openssh or sftp-server
dir_$(OPENSSH_ENABLED)					+= openssh

# dropbear
dir_$(DROPBEAR_ENABLED)					+= dropbear

# openvpn
dir_$(CONFIG_FIRMWARE_INCLUDE_OPENVPN)			+= openvpn

# xupnpd
dir_$(CONFIG_FIRMWARE_INCLUDE_XUPNPD)			+= xupnpd

# minidlna
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)			+= minidlna

# mt-daapd (firefly)
dir_$(CONFIG_FIRMWARE_INCLUDE_FIREFLY)			+= firefly

# transmission
dir_$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)		+= transmission

# aria2
dir_$(CONFIG_FIRMWARE_INCLUDE_ARIA)			+= aria2


all:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || \
		$(MAKE) -j1 -C $$i || \
		exit $$? ; \
	done

%_only:
	$(MAKE) -C $(@:_only=)

%_romfs:
	$(MAKE) -C $(@:_romfs=) romfs

%_clean:
	$(MAKE) -C $(@:_clean=) clean

romfs:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || \
		$(MAKE) -C $$i romfs ; \
	done

clean:
	for i in $(dir_y) $(dir_n) $(dir_) ; do \
		[ ! -d $$i ] || \
		$(MAKE) -C $$i clean ; \
	done
