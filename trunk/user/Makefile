.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean

ifndef ROOTDIR
ROOTDIR=..
endif
ifndef ROMFSDIR
ROMFSDIR=$(ROOTDIR)/romfs
endif

UCLINUX_BUILD_USER=1
-include $(CONFIG_CONFIG)
-include $(LINUX_CONFIG)
-include $(ARCH_CONFIG)

INSTALLDIR = $(ROOTDIR)/romfs

dir_y =
dir_n =
dir_  =

dir_y							+= busybox
ifeq ($(KERNEL3X),y)
dir_y							+= iptables-1.4.x
dir_$(CONFIG_FIRMWARE_INCLUDE_IPSET)			+= ipset
dir_y							+= pppoe-relay
else
dir_y							+= iptables-1.3.x
endif
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
dir_y							+= vsftpd
dir_y							+= samba3
dir_y							+= inadyn
dir_y							+= miniupnpd
dir_y							+= dnsmasq
dir_$(CONFIG_IPV6)					+= radvd
dir_y							+= igmpproxy
dir_y							+= udpxy
dir_y							+= nfsd
dir_y							+= util-linux
dir_y							+= e2fsprogs
dir_y							+= hdparm
dir_y							+= utils
dir_y							+= lanauth
dir_y							+= comgt-0.32
dir_y							+= 802.1x
dir_y							+= wpa_supplicant
dir_y							+= pppd
dir_y							+= accel-pptpd
dir_y							+= xl2tpd
dir_y							+= rp-l2tp
dir_y							+= usb-modeswitch
dir_y							+= tcpcheck
dir_y							+= dosfstools
dir_y							+= lldt
dir_y							+= LPRng
dir_y							+= u2ec
dir_y							+= p910nd
dir_y							+= optware
dir_y							+= scripts

# tcpdump
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)			+= tcpdump

# parted
dir_$(CONFIG_FIRMWARE_INCLUDE_PARTED)			+= parted

# openssh or sftp server
dir_$(if $(CONFIG_FIRMWARE_INCLUDE_OPENSSH)$(CONFIG_FIRMWARE_INCLUDE_SFTP),y,n)	+= openssh

# dropbear (if no openssh)
dir_$(if $(CONFIG_FIRMWARE_INCLUDE_OPENSSH),n,y)	+= dropbear

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
