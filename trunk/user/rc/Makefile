USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared
HTTPDIR = $(ROOTDIR)/user/httpd
INSTALLDIR = $(ROOTDIR)/romfs

CFLAGS += -s -I. -I$(SHDIR) -I$(SHDIR)/include -I$(HTTPDIR)
CFLAGS += $(if $(CONFIG_FIRMWARE_BUILDS_VER),-DFWBLDSTR=\"$(CONFIG_FIRMWARE_BUILDS_VER)\",)
CFLAGS += $(if $(CONFIG_FIRMWARE_USE_RPL2TP),-DUSE_RPL2TP,)
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)
CFLAGS += $(if $(CONFIG_RALINK_RT3883_3T3R),-DUSE_RT3883_3T3R,)
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)
ifeq ($(KERNEL3X),y)
CFLAGS += -DUSE_KERNEL3X
endif

# Disable/Enable WPS button event
CFLAGS += -DWPS_EVENT

# Disable/Enable ugly httpd_check
CFLAGS += -DHTTPD_CHECK

#CFLAGS += -DW7_LOGO	# win7 logo
#CFLAGS += -DWIFI_LOGO

LDFLAGS += -L. -lm

CFLAGS += -I$(USERDIR)/libdisk
CFLAGS += -I$(USERDIR)/wireless_tools 

LDFLAGS += -L$(USERDIR)/libdisk -ldisk
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
LDFLAGS += -L$(SHDIR) -lshared

EXEC = rc

OBJS  = rc.o init.o auth.o services.o watchdog.o firewall_ex.o common_ex.o 
OBJS += net.o net_lan.o net_wan.o net_wifi.o net_ppp.o services_ex.o 
OBJS += ralink.o rtl8367m.o detect_link.o detect_internet.o detect_wan.o usb_devices.o
ifdef CONFIG_IPV6
OBJS += net6.o net_lan6.o net_wan6.o net_ppp6.o
endif

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
	cd $(INSTALLDIR)/sbin && ln -sf rc watchdog
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc shutdown
	cd $(INSTALLDIR)/sbin && ln -sf rc halt
	cd $(INSTALLDIR)/sbin && ln -sf rc reboot
	cd $(INSTALLDIR)/sbin && ln -sf rc stopservice
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc start_ddns
	cd $(INSTALLDIR)/sbin && ln -sf rc getCountryCode
	cd $(INSTALLDIR)/sbin && ln -sf rc setCountryCode
	cd $(INSTALLDIR)/sbin && ln -sf rc gen_ralink_config
	cd $(INSTALLDIR)/sbin && ln -sf rc gen_ralink_config_rt
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dhcpd
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc run_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb1
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb2
	cd $(INSTALLDIR)/sbin && ln -sf rc start_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc run_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_firewall
	cd $(INSTALLDIR)/sbin && ln -sf rc rtl8367m
	cd $(INSTALLDIR)/sbin && ln -sf rc pids
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_networkmap
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_link
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_internet
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_wan
	cd $(INSTALLDIR)/sbin && ln -sf rc reset_to_defaults
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sg
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sd
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sr
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_lp
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_net
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_tty
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_usb
	if [ "$(CONFIG_FIRMWARE_INCLUDE_MINIDLNA)" = "y" ] ; then \
		cd $(INSTALLDIR)/sbin && ln -sf rc run_dms ; \
		cd $(INSTALLDIR)/sbin && ln -sf rc stop_dms ; \
	fi; \
	if [ "$(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION)" = "y" ] ; then \
		cd $(INSTALLDIR)/sbin && ln -sf rc run_torrent ; \
		cd $(INSTALLDIR)/sbin && ln -sf rc stop_torrent ; \
	fi;

