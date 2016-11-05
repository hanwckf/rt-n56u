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

FS_EXT_ENABLED=n
FS_FAT_ENABLED=n
FS_NTFS_ENABLED=n
STORAGE_ENABLED=n
OPENSSH_ENABLED=n
DROPBEAR_ENABLED=n
SAMBA_ENABLED=n
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

ifeq ($(CONFIG_FIRMWARE_INCLUDE_WINS),y)
SAMBA_ENABLED=y
endif

ifdef CONFIG_MMC_BLOCK
STORAGE_ENABLED=y
endif
ifdef CONFIG_BLK_DEV_SD
STORAGE_ENABLED=y
endif

ifeq ($(STORAGE_ENABLED),y)
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
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
SAMBA_ENABLED=y
endif
endif

dir_y =
dir_n =
dir_  =

dir_y						+= busybox
dir_y						+= iptables
dir_$(CONFIG_FIRMWARE_INCLUDE_IPSET)		+= ipset
dir_y						+= ebtables
dir_y						+= iproute2
dir_y						+= wireless_tools
dir_y						+= shared
dir_$(STORAGE_ENABLED)				+= libdisk
dir_y						+= nvram
dir_y						+= mtd_write
dir_y						+= networkmap
dir_y						+= httpd
dir_y						+= www
dir_y						+= rc
dir_y						+= infosvr
dir_y						+= inadyn
dir_y						+= miniupnpd
dir_y						+= dnsmasq
dir_y						+= igmpproxy
dir_y						+= udpxy
dir_y						+= utils
dir_y						+= lanauth
dir_y						+= 802.1x
dir_y						+= wpa_supplicant
dir_y						+= pppd
dir_y						+= pppoe
dir_y						+= accel-pptpd
dir_y						+= xl2tpd
dir_$(CONFIG_FIRMWARE_INCLUDE_RPL2TP)		+= rp-l2tp
dir_y						+= lldt
dir_y						+= scripts
dir_$(OPENSSH_ENABLED)				+= openssh
dir_$(DROPBEAR_ENABLED)				+= dropbear
dir_$(CONFIG_FIRMWARE_INCLUDE_OPENVPN)		+= openvpn
dir_$(CONFIG_FIRMWARE_INCLUDE_SSWAN)		+= strongswan
dir_$(CONFIG_FIRMWARE_INCLUDE_XUPNPD)		+= xupnpd
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)		+= tcpdump
dir_$(SAMBA_ENABLED)				+= samba3

ifdef CONFIG_BLK_DEV_SD
dir_$(CONFIG_FIRMWARE_INCLUDE_HDPARM)		+= hdparm
endif

ifdef CONFIG_USB_SUPPORT
dir_y						+= p910nd
dir_y						+= usb-modeswitch
dir_y						+= comgt-0.32
dir_y						+= uqmi
dir_$(CONFIG_FIRMWARE_INCLUDE_LPRD)		+= LPRng
dir_$(CONFIG_FIRMWARE_INCLUDE_U2EC)		+= u2ec
endif

ifeq ($(STORAGE_ENABLED),y)
dir_y						+= nfsd
dir_y						+= util-linux
dir_y						+= optware
dir_$(FS_EXT_ENABLED)				+= e2fsprogs
dir_$(FS_FAT_ENABLED)				+= dosfstools
dir_$(FS_NTFS_ENABLED)				+= ntfs-3g
dir_$(CONFIG_FIRMWARE_INCLUDE_PARTED)		+= parted
dir_$(CONFIG_FIRMWARE_INCLUDE_FTPD)		+= vsftpd
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)		+= minidlna
dir_$(CONFIG_FIRMWARE_INCLUDE_FIREFLY)		+= firefly
dir_$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)	+= transmission
dir_$(CONFIG_FIRMWARE_INCLUDE_ARIA)		+= aria2
endif

ifdef CONFIG_MTD_UBI
ifneq ($(STORAGE_ENABLED),y)
dir_y						+= util-linux
dir_y						+= optware
endif
dir_y						+= mtd-utils
endif

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
	for i in `ls -d *` ; do \
		[ ! -d $$i ] || \
		$(MAKE) -C $$i clean ; \
	done
