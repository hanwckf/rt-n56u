#
# Router rc control scripts Makefile
#
# Copyright 2004, ASUSTeK Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#

-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared
HTTPDIR = $(ROOTDIR)/user/httpd
INSTALLDIR = $(ROOTDIR)/romfs

FWBLDV = $(if $(CONFIG_FIRMWARE_BUILDS_VER),-DFWBLDSTR=\"$(CONFIG_FIRMWARE_BUILDS_VER)\",)
RPL2TP = $(if $(CONFIG_FIRMWARE_USE_RPL2TP),-DUSE_RPL2TP,)
PHYMAC = $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)
RT3883 = $(if $(CONFIG_RALINK_RT3883_3T3R),-DUSE_RT3883_3T3R,)
DOIPV6 = $(if $(CONFIG_IPV6),-DUSE_IPV6,)

CFLAGS += -s -DASUS_NVRAM -DWL500GP -DASUS_EXT -DASUS_DDNS -DWEBSTRFILTER -DU2EC
CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include -I$(HTTPDIR)
CFLAGS += $(FWBLDV)
CFLAGS += $(RPL2TP)
CFLAGS += $(PHYMAC)
CFLAGS += $(RT3883)
CFLAGS += $(DOIPV6)

# Disable/Enable WPS button event
CFLAGS += -DWPS_EVENT

# Disable/Enable ugly httpd_check
CFLAGS += -DHTTPD_CHECK

#CFLAGS += -DLANGUAGE_TW
CFLAGS += -DSR3
#CFLAGS += -DW7_LOGO	# win7 logo
#CFLAGS += -DWIFI_LOGO
#CFLAGS += -DMR

LDFLAGS += -L. -lm

CFLAGS += -I$(USERDIR)/libdisk
CFLAGS += -I$(USERDIR)/wireless_tools 
#CFLAGS += -I$(LINUXINCDIR)

LDFLAGS += -L$(USERDIR)/libdisk -ldisk
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
LDFLAGS += -L$(SHDIR) -lshared

EXEC = rc

OBJS  = rc.o init.o auth.o services.o watchdog.o network_ex.o firewall_ex.o common_ex.o network.o services_ex.o ppp.o 
OBJS += ntp.o udhcpc.o udhcpc_ex.o ralink.o rtl8367m.o linkstatus.o detect_internet.o detect_wan.o usb_devices.o


all:	$(OBJS) Makefile
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

c.o:
	$(CC) -c $*.c $(CFLAGS)

clean:
	rm -f *.o rc

romfs:
	$(ROMFSINST) /sbin/$(EXEC)
	cd $(INSTALLDIR) && rm -f init && ln -sf sbin/rc init
	cd $(INSTALLDIR)/sbin && ln -sf rc init
	cd $(INSTALLDIR)/sbin && ln -sf rc nvram_restore
	cd $(INSTALLDIR)/sbin && ln -sf rc nvram_erase
	cd $(INSTALLDIR)/sbin && ln -sf rc start_mac_clone
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc shutdown
	cd $(INSTALLDIR)/sbin && ln -sf rc halt
	cd $(INSTALLDIR)/sbin && ln -sf rc reboot
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservice
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc start_ddns
	cd $(INSTALLDIR)/sbin && ln -sf rc ntp
	cd $(INSTALLDIR)/sbin && ln -sf rc gpio_write
	cd $(INSTALLDIR)/sbin && ln -sf rc ip-up
	cd $(INSTALLDIR)/sbin && ln -sf rc ip-down
	cd $(INSTALLDIR)/sbin && ln -sf rc wan-up
	cd $(INSTALLDIR)/sbin && ln -sf rc wan-down
	cd $(INSTALLDIR)/sbin && ln -sf rc getCountryCode
	cd $(INSTALLDIR)/sbin && ln -sf rc setCountryCode
	cd $(INSTALLDIR)/sbin && ln -sf rc getMAC
	cd $(INSTALLDIR)/sbin && ln -sf rc setMAC
	cd $(INSTALLDIR)/sbin && ln -sf rc getMAC_2G
	cd $(INSTALLDIR)/sbin && ln -sf rc setMAC_2G
	cd $(INSTALLDIR)/sbin && ln -sf rc getMAC_5G
	cd $(INSTALLDIR)/sbin && ln -sf rc setMAC_5G
	cd $(INSTALLDIR)/sbin && ln -sf rc getPIN
	cd $(INSTALLDIR)/sbin && ln -sf rc setPIN
	cd $(INSTALLDIR)/sbin && ln -sf rc getBootV
	cd $(INSTALLDIR)/sbin && ln -sf rc gen_ralink_config
	cd $(INSTALLDIR)/sbin && ln -sf rc gen_ralink_config_rt
	cd $(INSTALLDIR)/sbin && ln -sf rc slink
	cd $(INSTALLDIR)/sbin && ln -sf rc getSSID
	cd $(INSTALLDIR)/sbin && ln -sf rc getChannel
	cd $(INSTALLDIR)/sbin && ln -sf rc getChannel_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc asuscfe
	cd $(INSTALLDIR)/sbin && ln -sf rc asuscfe_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc asuscfe_5g
	cd $(INSTALLDIR)/sbin && ln -sf rc getSiteSurvey
	cd $(INSTALLDIR)/sbin && ln -sf rc getSiteSurvey_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc getBSSID
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dhcpd
	cd $(INSTALLDIR)/sbin && ln -sf rc convert_asus_values
	cd $(INSTALLDIR)/sbin && ln -sf rc FWRITE
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc run_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc umount2
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb1
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb2
	cd $(INSTALLDIR)/sbin && ln -sf rc start_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_firewall
	cd $(INSTALLDIR)/sbin && ln -sf rc start_ntp
	cd $(INSTALLDIR)/sbin && ln -sf rc get_sw
	cd $(INSTALLDIR)/sbin && ln -sf rc ledon
	cd $(INSTALLDIR)/sbin && ln -sf rc ledoff
	cd $(INSTALLDIR)/sbin && ln -sf rc rtl8367m
	cd $(INSTALLDIR)/sbin && ln -sf rc linkstatus_monitor
	cd $(INSTALLDIR)/sbin && ln -sf rc pids
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_networkmap
	cd $(INSTALLDIR)/sbin && ln -sf rc dumparptable
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_internet
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_wan
	cd $(INSTALLDIR)/sbin && ln -sf rc stainfo
	cd $(INSTALLDIR)/sbin && ln -sf rc stainfo_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc getstat
	cd $(INSTALLDIR)/sbin && ln -sf rc getstat_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc getrssi
	cd $(INSTALLDIR)/sbin && ln -sf rc getrssi_2g
	cd $(INSTALLDIR)/sbin && ln -sf rc getrssi_5g
	cd $(INSTALLDIR)/sbin && ln -sf rc gettxbfcal
	cd $(INSTALLDIR)/sbin && ln -sf rc reset_to_defaults
	cd $(INSTALLDIR)/sbin && ln -sf rc refresh_ntpc
	cd $(INSTALLDIR)/sbin && ln -sf rc run_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc umount_dev_all
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_lp
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_sd
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_sg
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_sr
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_net
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_tty
	cd $(INSTALLDIR)/sbin && ln -sf rc asus_usb_interface ;
	if [ "$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)" = "y" ] ; then \
		cd $(INSTALLDIR)/sbin && ln -sf rc run_dms ; \
		cd $(INSTALLDIR)/sbin && ln -sf rc stop_dms ; \
	fi; \
	if [ "$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)" = "y" ] ; then \
		cd $(INSTALLDIR)/sbin && ln -sf rc run_torrent ; \
		cd $(INSTALLDIR)/sbin && ln -sf rc stop_torrent ; \
	fi; 
	
