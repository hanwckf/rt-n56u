ifndef ROOTDIR
ROOTDIR=../..
endif

USERDIR = $(ROOTDIR)/user
SHDIR = $(ROOTDIR)/user/shared
INSTALLDIR = $(ROOTDIR)/romfs

include $(SHDIR)/boards.mk
include $(SHDIR)/cflags.mk

CFLAGS += -s -I. -I$(SHDIR) -I$(SHDIR)/include
CFLAGS += -I$(USERDIR)/wireless_tools
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include/ralink

# Disable/Enable ugly httpd_check
#CFLAGS += -DHTTPD_CHECK

LDFLAGS += -L. -lm
LDFLAGS += -L$(USERDIR)/wireless_tools -liw
LDFLAGS += -L$(SHDIR) -lshared
ifeq ($(STORAGE_ENABLED),y)
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
ifeq ($(STORAGE_ENABLED),y)
OBJS += hotplug_stor.o
OBJS += services_stor.o
endif
ifdef CONFIG_USB_SUPPORT
OBJS += hotplug_usb.o usb_modem.o services_usb.o
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
ifdef CONFIG_SMP
OBJS += smp.o
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
	cd $(INSTALLDIR)/sbin && ln -sf rc mtk_gpio
	cd $(INSTALLDIR)/sbin && ln -sf rc ddns_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc ntpc_updated
	cd $(INSTALLDIR)/sbin && ln -sf rc start_ddns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dns
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_dhcpd
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_vpn_server
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_vpn_client
	cd $(INSTALLDIR)/sbin && ln -sf rc run_telnetd
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_firewall
	cd $(INSTALLDIR)/sbin && ln -sf rc pids
	cd $(INSTALLDIR)/sbin && ln -sf rc rstats
	cd $(INSTALLDIR)/sbin && ln -sf rc stop_wan
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_wan
	cd $(INSTALLDIR)/sbin && ln -sf rc restart_networkmap
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_link
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_internet
	cd $(INSTALLDIR)/sbin && ln -sf rc detect_wan
	cd $(INSTALLDIR)/sbin && ln -sf rc reset_to_defaults
	cd $(INSTALLDIR)/sbin && ln -sf rc lan_eeprom_mac
	cd $(INSTALLDIR)/sbin && ln -sf rc wan_eeprom_mac
	cd $(INSTALLDIR)/sbin && ln -sf rc leds_front
	cd $(INSTALLDIR)/sbin && ln -sf rc leds_ether
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
ifdef CONFIG_RTL8367
	cd $(INSTALLDIR)/sbin && ln -sf rc rtl8367
endif
ifdef CONFIG_RAETH_ESW
	cd $(INSTALLDIR)/sbin && ln -sf rc mtk_esw
endif
ifdef CONFIG_MT7530_GSW
	cd $(INSTALLDIR)/sbin && ln -sf rc mtk_esw
endif
ifdef CONFIG_RT3352_INIC_MII
	cd $(INSTALLDIR)/sbin && ln -sf rc inicd
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENVPN),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc ovpn_export_client
endif

ifeq ($(STORAGE_ENABLED),y)
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
endif
ifeq ($(STORAGE_ENABLED),y)
	cd $(INSTALLDIR)/sbin && ln -sf rc ejall
endif
ifdef CONFIG_ATA
	cd $(INSTALLDIR)/sbin && ln -sf rc ejata
endif
ifdef CONFIG_MMC_BLOCK
	cd $(INSTALLDIR)/sbin && ln -sf rc ejmmc
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_mmc
endif
ifdef CONFIG_BLK_DEV_SD
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sd
endif
ifdef CONFIG_USB_SUPPORT
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sr
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_sg
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_lp
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_net
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_tty
	cd $(INSTALLDIR)/sbin && ln -sf rc mdev_wdm
	cd $(INSTALLDIR)/sbin && ln -sf rc zerocd
	cd $(INSTALLDIR)/sbin && ln -sf rc usb5v
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb1
ifneq ($(BOARD_NUM_USB_PORTS),1)
	cd $(INSTALLDIR)/sbin && ln -sf rc ejusb2
endif
endif
