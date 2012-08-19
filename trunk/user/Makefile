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
dir_y							+= bridge-utils
dir_y							+= iptables
dir_y							+= wireless_tools
dir_y							+= shared
dir_y							+= libdisk
dir_y							+= nvram
dir_y							+= mtd_write
dir_y							+= ntpclient.asus
dir_y							+= networkmap
dir_y							+= httpd
dir_y							+= www
dir_y							+= rc
dir_y							+= vsftpd
dir_y							+= samba3
dir_y							+= inadyn
dir_y							+= ez-ipupdate
dir_y							+= miniupnpd
dir_y							+= dnsmasq
dir_y							+= igmpproxy
dir_y							+= udpxy
dir_y							+= nfsd
dir_y							+= e2fsprogs
dir_y							+= hdparm
dir_y							+= sdparm
dir_y							+= utils
dir_y							+= lanauth
dir_y							+= comgt-0.32
dir_y							+= 802.1x
dir_y							+= wpa_supplicant
dir_y							+= pppd
dir_y							+= accel-pptpd
dir_y							+= xl2tpd
dir_$(CONFIG_FIRMWARE_USE_RPL2TP)			+= rp-l2tp
dir_y							+= usb_modeswitch
dir_y							+= rstats
dir_y							+= tcpcheck
dir_y							+= dosfstools
dir_y							+= lldt
dir_y							+= iproute2
dir_y							+= LPRng
dir_y							+= u2ec
dir_y							+= p910nd
dir_y							+= optware
dir_y							+= scripts

# tcpdump
dir_$(CONFIG_FIRMWARE_INCLUDE_TCPDUMP)			+= tcpdump

# openssh or sftp server
dir_$(if $(CONFIG_FIRMWARE_INCLUDE_OPENSSH)$(CONFIG_FIRMWARE_INCLUDE_SFTP),y,n)	+= openssh

# dropbear (if no openssh)
dir_$(if $(CONFIG_FIRMWARE_INCLUDE_OPENSSH),n,y)	+= dropbear

# minidlna
dir_$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)			+= minidlna

# transmission
dir_$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)		+= transmission


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
