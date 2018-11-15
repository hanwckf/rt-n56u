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
TTYD_ENABLED=n
HTOP_ENABLE=n
SAMBA_ENABLED=n
CHNROUTE_ENABLED=n
ARIA2_ENABLED=n
SOFTETHERVPN_ENABLE=n
SHADOWSOCKS_ENABLE=n

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

ifeq ($(CONFIG_FIRMWARE_INCLUDE_TTYD),y)
TTYD_ENABLED=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTOP),y)
HTOP_ENABLE=y
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

ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA),y)
ARIA2_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN),y)
ARIA2_ENABLED=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
CHNROUTE_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_CHINADNS),y)
CHNROUTE_ENABLED=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CMD),y)
SOFTETHERVPN_ENABLE=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CLIENT),y)
SOFTETHERVPN_ENABLE=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SSSERVER),y)
SHADOWSOCKS_ENABLE=y
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)
SHADOWSOCKS_ENABLE=y
endif

dir_y =
dir_n =
dir_  =

dir_y						+= busybox
dir_y						+= iptables
ifneq ($(CONFIG_WITHOUT_KERNEL),y)
dir_$(CONFIG_FIRMWARE_INCLUDE_IPSET)		+= ipset
dir_y						+= accel-pptpd
endif
dir_y						+= ebtables
dir_y						+= iproute2
ifneq ($(CONFIG_WITHOUT_IW),y)
dir_y						+= wireless_tools
endif
dir_y						+= shared
dir_$(STORAGE_ENABLED)				+= libdisk
dir_y						+= nvram
dir_y						+= mtd_write
dir_y						+= networkmap
dir_y						+= httpd
dir_y						+= www
dir_y						+= rc
dir_$(CONFIG_FIRMWARE_INCLUDE_NAPT66)		+= napt66
dir_$(CONFIG_FIRMWARE_INCLUDE_SFE)		+= shortcut-fe
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
dir_y						+= xl2tpd
dir_$(CONFIG_FIRMWARE_INCLUDE_RPL2TP)		+= rp-l2tp
dir_y						+= lldt
dir_y						+= scripts
dir_$(OPENSSH_ENABLED)				+= openssh
dir_$(DROPBEAR_ENABLED)				+= dropbear
dir_$(HTOP_ENABLE)					+= htop
dir_$(CONFIG_FIRMWARE_INCLUDE_NANO)		+= nano
dir_$(CONFIG_FIRMWARE_INCLUDE_OPENVPN)		+= openvpn
dir_$(CONFIG_FIRMWARE_INCLUDE_SSWAN)		+= strongswan
dir_$(CONFIG_FIRMWARE_INCLUDE_XUPNPD)		+= xupnpd
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)		+= tcpdump
dir_$(CONFIG_FIRMWARE_INCLUDE_MTR)		+= mtr
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD36),y)
dir_$(SAMBA_ENABLED)				+= samba36
else
dir_$(SAMBA_ENABLED)				+= samba3
endif
dir_$(CHNROUTE_ENABLED)				+= chnroute
dir_$(TTYD_ENABLED)					+= ttyd
dir_$(SOFTETHERVPN_ENABLE)			+= softethervpn
dir_$(CONFIG_FIRMWARE_INCLUDE_SCUTCLIENT)         += scutclient
dir_$(CONFIG_FIRMWARE_INCLUDE_VLMCSD)			+= vlmcsd
dir_$(CONFIG_FIRMWARE_INCLUDE_CHINADNS)		+= chinadns
dir_$(CONFIG_FIRMWARE_INCLUDE_DNSFORWARDER)		+= dns-forwarder
dir_$(SHADOWSOCKS_ENABLE)				+= shadowsocks
dir_$(CONFIG_FIRMWARE_INCLUDE_LRZSZ)			+= lrzsz
dir_$(CONFIG_FIRMWARE_INCLUDE_GDUT_DRCOM)		+= gdut-drcom
dir_$(CONFIG_FIRMWARE_INCLUDE_DNSMASQ_CHINA_CONF)	+= dnsmasq-china-conf
dir_$(CONFIG_FIRMWARE_INCLUDE_DOGCOM)			+= dogcom
dir_$(CONFIG_FIRMWARE_INCLUDE_IPERF)			+= iperf
dir_$(CONFIG_FIRMWARE_INCLUDE_IPERF3)			+= iperf3
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIEAP)			+= minieap
dir_$(CONFIG_FIRMWARE_INCLUDE_NJIT_CLIENT)		+= njit-client

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
dir_$(CONFIG_FIRMWARE_INCLUDE_DUMP1090)	+= dump1090
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
dir_$(ARIA2_ENABLED)		+= aria2
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
