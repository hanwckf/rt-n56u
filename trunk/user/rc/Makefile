include ../shared/boards.mk
include ../shared/cflags.mk

USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared
INSTALLDIR = $(ROOTDIR)/romfs

CFLAGS += -s -I. -I$(SHDIR) -I$(SHDIR)/include
CFLAGS += -I$(USERDIR)/wireless_tools

# Disable/Enable ugly httpd_check
#CFLAGS += -DHTTPD_CHECK

LDFLAGS += -L. -lm
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
LDFLAGS += -L$(SHDIR) -lshared
ifdef CONFIG_USB_SUPPORT
CFLAGS += -I$(USERDIR)/libdisk
LDFLAGS += -L$(USERDIR)/libdisk -ldisk
endif

EXEC = rc

OBJS  = rc.o init.o auth.o services.o watchdog.o firewall_ex.o common_ex.o
OBJS += net.o net_lan.o net_wan.o net_wifi.o net_ppp.o services_ex.o rstats.o
OBJS += ralink.o gpio_pins.o detect_link.o detect_internet.o detect_wan.o
OBJS += vpn_server.o vpn_client.o
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENVPN),y)
OBJS += vpn_openvpn.o
endif
ifdef CONFIG_USB_SUPPORT
OBJS += usb_devices.o usb_modem.o services_usb.o
endif
ifdef CONFIG_RTL8367
OBJS += switch_rtl8367.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/rtl8367
else
ifdef CONFIG_MT7530_GSW
OBJS += switch_mtk_esw.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth
else
ifdef CONFIG_RAETH_ESW
OBJS += switch_mtk_esw.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth
endif
endif
endif
ifdef CONFIG_RT3352_INIC_MII
OBJS += inicd.o
endif
ifdef CONFIG_IPV6
OBJS += net6.o net_lan6.o net_wan6.o
endif

all: $(OBJS) Makefile
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
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_restart
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle_off
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_toggle_on
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_guest_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_guest_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio2_eeprom_mac
ifeq ($(BOARD_HAS_5G_RADIO),1)
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_restart
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle_off
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_toggle_on
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_guest_enable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_guest_disable
	cd $(INSTALLDIR)/sbin && ln -sf rc radio5_eeprom_mac
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
ifdef CONFIG_USB_SUPPORT
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sg
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sd
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sr
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_lp
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_net
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_tty
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_wdm
	cd $(INSTALLDIR)/sbin && ln -sf rc zerocd
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb1
ifeq ($(BOARD_NUM_USB_PORTS),2)
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb2
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_MINIDLNA),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_minidlna
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_minidlna
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_firefly
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_firefly
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_TRANSMISSION),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_transmission
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_transmission
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_ARIA),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_aria
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_aria
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_samba
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_FTPD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftp
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftp
ifneq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_ftpsamba
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_ftpsamba
endif
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NFSD),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc run_nfsd
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_nfsd
endif
endif
