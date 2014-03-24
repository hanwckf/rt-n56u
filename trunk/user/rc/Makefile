USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared
HTTPDIR = $(ROOTDIR)/user/httpd
INSTALLDIR = $(ROOTDIR)/romfs

CFLAGS += -s -I. -I$(SHDIR) -I$(SHDIR)/include -I$(HTTPDIR)
CFLAGS += $(if $(CONFIG_FIRMWARE_BUILDS_VER),-DFWBLDSTR=\"$(CONFIG_FIRMWARE_BUILDS_VER)\",)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_LPRD),-DSRV_LPRD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_U2EC),-DSRV_U2EC,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_OPENVPN),-DAPP_OPENVPN,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_XUPNPD),-DAPP_XUPNPD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_MINIDLNA),-DAPP_MINIDLNA,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_FIREFLY),-DAPP_FIREFLY,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION),-DAPP_TRMD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_ARIA),-DAPP_ARIA,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_SMBD),-DAPP_SMBD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_FTPD),-DAPP_FTPD,)
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_NFSD),-DAPP_NFSD,)
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)
CFLAGS += $(if $(CONFIG_NETFILTER_XT_MATCH_STATE),,-DUSE_MATCH_CONNTRACK)
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)
CFLAGS += $(if $(CONFIG_USB_XHCI_HCD),-DUSE_USB3,)
CFLAGS += $(if $(CONFIG_RT2860V2_AP),-DUSE_RT2860V2_AP,)
CFLAGS += $(if $(CONFIG_RT3090_AP),-DUSE_RT3090_AP,)
CFLAGS += $(if $(CONFIG_RT5392_AP),-DUSE_RT5392_AP,)
CFLAGS += $(if $(CONFIG_RT5592_AP),-DUSE_RT5592_AP,)
CFLAGS += $(if $(CONFIG_RT3593_AP),-DUSE_RT3593_AP,)
CFLAGS += $(if $(CONFIG_RT3352_INIC_MII),-DUSE_RT3352_MII,)
CFLAGS += $(if $(CONFIG_HNAT_V2),-DUSE_HW_NAT_V2,)
CFLAGS += $(if $(CONFIG_RA_HW_NAT_IPV6),-DUSE_IPV6_HW_NAT,)
CFLAGS += $(if $(CONFIG_RA_HW_NAT_PCI),-DUSE_WWAN_HW_NAT,)

ifdef CONFIG_RALINK_RT3883
CFLAGS += -DCONFIG_RALINK_RT3883
else
ifdef CONFIG_RALINK_MT7620
CFLAGS += -DCONFIG_RALINK_MT7620
CFLAGS += -DAP_MODE_LAN_TAGGED
endif
endif

ifdef CONFIG_RTL8367
CFLAGS += -DUSE_RTL8367
CFLAGS += $(if $(CONFIG_RTL8367_API_8367B),-DUSE_RTL8367_API_8367B,)
CFLAGS += $(if $(CONFIG_RTL8367_IGMP_SNOOPING),-DUSE_RTL8367_IGMP_SNOOPING,)
else
ifdef CONFIG_RAETH_ESW
CFLAGS += -DUSE_MTK_ESW
endif
endif

ifneq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSH),y)
ifeq ($(CONFIG_FIRMWARE_INCLUDE_DROPBEAR),y)
CFLAGS += -DAPP_SSHD
endif
else
CFLAGS += -DAPP_SSHD
endif

ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
CFLAGS += -DSUPPORT_HTTPS
endif

# Disable/Enable ugly httpd_check
#CFLAGS += -DHTTPD_CHECK
#CFLAGS += -DW7_LOGO
#CFLAGS += -DWIFI_LOGO

LDFLAGS += -L. -lm

CFLAGS += -I$(USERDIR)/libdisk
CFLAGS += -I$(USERDIR)/wireless_tools

LDFLAGS += -L$(USERDIR)/libdisk -ldisk
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
LDFLAGS += -L$(SHDIR) -lshared

EXEC = rc

OBJS  = rc.o init.o auth.o services.o watchdog.o firewall_ex.o common_ex.o
OBJS += net.o net_lan.o net_wan.o net_wifi.o net_ppp.o services_ex.o rstats.o
OBJS += ralink.o detect_link.o detect_internet.o detect_wan.o usb_devices.o usb_modem.o
OBJS += vpn_server.o vpn_client.o
ifdef CONFIG_FIRMWARE_INCLUDE_OPENVPN
OBJS += vpn_openvpn.o
endif
ifdef CONFIG_RTL8367
OBJS += switch_rtl8367.o
else
ifdef CONFIG_RAETH_ESW
OBJS += switch_mtk_esw.o
endif
endif
ifdef CONFIG_RT3352_INIC_MII
OBJS += inicd.o
endif
ifdef CONFIG_IPV6
OBJS += net6.o net_lan6.o net_wan6.o
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
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc ntpc_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc start_ddns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dhcpd
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb1
	cd $(INSTALLDIR)/sbin && ln -sf rc start_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc run_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_firewall
	cd $(INSTALLDIR)/sbin && ln -sf rc pids
	cd $(INSTALLDIR)/sbin && ln -sf rc rstats
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_wan
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
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_wdm
	cd $(INSTALLDIR)/sbin && ln -sf rc zerocd
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle_off
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle_on
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_eeprom_mac
ifneq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N14U")
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle_off
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle_on
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_eeprom_mac
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb2
endif
ifdef CONFIG_RAETH_ESW
	cd $(INSTALLDIR)/sbin && ln -sf rc mtk_esw
endif
ifdef CONFIG_RTL8367
	cd $(INSTALLDIR)/sbin && ln -sf rc rtl8367
endif
ifdef CONFIG_RT3352_INIC_MII
	cd $(INSTALLDIR)/sbin && ln -sf rc inicd
endif
ifdef CONFIG_FIRMWARE_INCLUDE_MINIDLNA
	cd $(INSTALLDIR)/sbin && ln -sf rc run_minidlna
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_minidlna
endif
ifdef CONFIG_FIRMWARE_INCLUDE_FIREFLY
	cd $(INSTALLDIR)/sbin && ln -sf rc run_firefly
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_firefly
endif
ifdef CONFIG_FIRMWARE_INCLUDE_TRANSMISSION
	cd $(INSTALLDIR)/sbin && ln -sf rc run_transmission
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_transmission
endif
ifdef CONFIG_FIRMWARE_INCLUDE_ARIA
	cd $(INSTALLDIR)/sbin && ln -sf rc run_aria
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_aria
endif
ifdef CONFIG_FIRMWARE_INCLUDE_SMBD
	cd $(INSTALLDIR)/sbin && ln -sf rc run_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
endif
ifdef CONFIG_FIRMWARE_INCLUDE_FTPD
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftp
ifndef CONFIG_FIRMWARE_INCLUDE_SMBD
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
endif
endif
ifdef CONFIG_FIRMWARE_INCLUDE_NFSD
	cd $(INSTALLDIR)/sbin && ln -sf rc run_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_nfsd
endif

