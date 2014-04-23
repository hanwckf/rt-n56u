/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _netutils_h_
#define _netutils_h_

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ralink_boards.h>

#if ((__UCLIBC_MAJOR__ == 0) && (__UCLIBC_MINOR__ < 9 || (__UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ < 30)))
#undef HAVE_GETIFADDRS
#else
#define HAVE_GETIFADDRS 1
#endif

#define IFNAME_BR			"br0"

#define IFNAME_MAC			"eth2"
#define IFNAME_MAC2			"eth3"
#if defined (USE_SINGLE_MAC)
#define IFNAME_LAN			"eth2.1"
#define IFNAME_WAN			"eth2.2"
#else
#define IFNAME_LAN			IFNAME_MAC
#define IFNAME_WAN			IFNAME_MAC2
#endif

#if BOARD_2G_IN_SOC
#define IFNAME_2G_MAIN			"ra0"
#define IFNAME_2G_GUEST			"ra1"
#define IFNAME_2G_APCLI			"apcli0"
#define IFNAME_2G_WDS0			"wds0"
#define IFNAME_2G_WDS1			"wds1"
#define IFNAME_2G_WDS2			"wds2"
#define IFNAME_2G_WDS3			"wds3"
#elif !BOARD_5G_IN_SOC
#define IFNAME_2G_MAIN			"rai2"
#define IFNAME_2G_GUEST			"rai3"
#define IFNAME_2G_APCLI			"apclii1"
#define IFNAME_2G_WDS0			"wdsi4"
#define IFNAME_2G_WDS1			"wdsi5"
#define IFNAME_2G_WDS2			"wdsi6"
#define IFNAME_2G_WDS3			"wdsi7"
#else
#define IFNAME_2G_MAIN			"rai0"
#define IFNAME_2G_GUEST			"rai1"
#define IFNAME_2G_APCLI			"apclii0"
#define IFNAME_2G_WDS0			"wdsi0"
#define IFNAME_2G_WDS1			"wdsi1"
#define IFNAME_2G_WDS2			"wdsi2"
#define IFNAME_2G_WDS3			"wdsi3"
#endif

#if BOARD_HAS_5G_RADIO
#if BOARD_5G_IN_SOC
#define IFNAME_5G_MAIN			"ra0"
#define IFNAME_5G_GUEST			"ra1"
#define IFNAME_5G_APCLI			"apcli0"
#define IFNAME_5G_WDS0			"wds0"
#define IFNAME_5G_WDS1			"wds1"
#define IFNAME_5G_WDS2			"wds2"
#define IFNAME_5G_WDS3			"wds3"
#else
#define IFNAME_5G_MAIN			"rai0"
#define IFNAME_5G_GUEST			"rai1"
#define IFNAME_5G_APCLI			"apclii0"
#define IFNAME_5G_WDS0			"wdsi0"
#define IFNAME_5G_WDS1			"wdsi1"
#define IFNAME_5G_WDS2			"wdsi2"
#define IFNAME_5G_WDS3			"wdsi3"
#endif
#endif

#if defined(USE_RT3352_MII)
#define IFNAME_INIC_MAIN		IFNAME_2G_MAIN
#define IFNAME_INIC_GUEST		IFNAME_2G_GUEST
#define IFNAME_INIC_GUEST_VLAN		"eth2.3"
#define INIC_GUEST_VLAN_VID		3
#define MIN_EXT_VLAN_VID		4
#else
#define MIN_EXT_VLAN_VID		3
#endif

#define IFNAME_SIT			"sit1"

#define WAN_PPP_UNIT			0
#define WAN_PPP_UNIT_MAX		1
#define IFNAME_PPP			"ppp0"

#define RAS_PPP_UNIT			2
#define IFNAME_RAS			"ppp2"

#define VPNC_PPP_UNIT			5
#define IFNAME_CLIENT_PPP		"ppp5"

#define IFNAME_SERVER_TAP		"tap1"
#define IFNAME_CLIENT_TAP		"tap0"

#define IFNAME_SERVER_TUN		"tun1"
#define IFNAME_CLIENT_TUN		"tun0"

#define IFNAME_USBNET1			"wwan0"
#define IFNAME_USBNET2			"weth0"

#define IFDESC_WAN			"WAN"
#define IFDESC_LAN			"LAN"
#define IFDESC_WLAN_2G			"WLAN2"
#define IFDESC_WLAN_5G			"WLAN5"
#define IFDESC_WWAN			"WWAN"
#define IFDESCS_MAX_NUM			5

enum {
	IPV6_DISABLED = 0,
	IPV6_NATIVE_STATIC,
	IPV6_NATIVE_DHCP6,
	IPV6_6IN4,
	IPV6_6TO4,
	IPV6_6RD
};

extern in_addr_t inet_addr_safe(const char *cp);
extern int       is_valid_ipv4(const char *cp);
extern int       is_valid_hostname(const char *hname);
extern int       get_ap_mode(void);
extern int       get_wan_phy_connected(void);
extern int       get_usb_modem_wan(int unit);
extern int       get_usb_modem_dev_wan(int unit, int devnum);
extern void      set_usb_modem_dev_wan(int unit, int devnum);

extern int  get_ipv6_type(void);

extern const char* get_ifname_descriptor(const char* ifname);

#if defined (USE_IPV6)
extern char *get_ifaddr6(char *ifname, int linklocal, char *p_addr6s);
#endif

#endif
