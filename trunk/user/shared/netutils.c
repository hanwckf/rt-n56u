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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <linux/sockios.h>
#include <linux/if_vlan.h>
#include <linux/if_bridge.h>

#include "nvram_linux.h"
#include "netutils.h"
#include "shutils.h"

#if defined (HAVE_GETIFADDRS)
#include <ifaddrs.h>
#endif

static const struct ifname_desc_t {
	const char *ifname;
	const char *ifdesc;
	unsigned char is_dynamic;
	unsigned char is_ap_mode;
	unsigned char is_router_mode;
	unsigned char wan_no;
} ifname_descs[] = {
	/*IFNAME             IFDESC           DY AM RM WAN*/
	{ IFNAME_WAN,        IFDESC_WAN,       0, 0, 1, 1 },
#if defined(USE_SINGLE_MAC) && !defined(AP_MODE_LAN_TAGGED)
	{ IFNAME_LAN,        IFDESC_LAN,       0, 0, 1, 0 },
	{ IFNAME_MAC,        IFDESC_LAN,       0, 1, 0, 0 },
#else
	{ IFNAME_LAN,        IFDESC_LAN,       0, 0, 0, 0 },
#endif
#if defined(USE_RT3352_MII)
	{ IFNAME_2G_MAIN,    IFDESC_WLAN2_MII, 0, 0, 0, 0 },
#else
	{ IFNAME_2G_MAIN,    IFDESC_WLAN2_AP0, 0, 0, 0, 0 },
	{ IFNAME_2G_GUEST,   IFDESC_WLAN2_AP1, 0, 0, 0, 0 },
	{ IFNAME_2G_APCLI,   IFDESC_WLAN2_APC, 0, 0, 0, 2 },
	{ IFNAME_2G_WDS0,    IFDESC_WLAN2_WDS, 0, 0, 0, 0 },
#endif
#if BOARD_HAS_5G_RADIO
	{ IFNAME_5G_MAIN,    IFDESC_WLAN5_AP0, 0, 0, 0, 0 },
	{ IFNAME_5G_GUEST,   IFDESC_WLAN5_AP1, 0, 0, 0, 0 },
	{ IFNAME_5G_APCLI,   IFDESC_WLAN5_APC, 0, 0, 0, 2 },
	{ IFNAME_5G_WDS0,    IFDESC_WLAN5_WDS, 0, 0, 0, 0 },
#endif
#if defined(USE_USB_SUPPORT)
	{ IFNAME_RAS,        IFDESC_WWAN,      1, 0, 1, 3 },
	{ IFNAME_USBNET1,    IFDESC_WWAN,      1, 0, 1, 3 },
	{ IFNAME_USBNET2,    IFDESC_WWAN,      1, 0, 1, 3 },
#endif
	{ IFNAME_CLIENT_PPP, IFDESC_VPNC,      1, 0, 1, 0 },
	{ IFNAME_CLIENT_TAP, IFDESC_VPNC,      1, 0, 1, 0 },
	{ IFNAME_CLIENT_TUN, IFDESC_VPNC,      1, 0, 1, 0 },

	{ NULL, NULL, 0, 0, 0, 0 }
};

const char*
get_ifname_descriptor(const char* ifname, int ap_mode, int *ifindex, int *wan_no)
{
	struct ifname_desc_t *ifd;

	for (ifd = (struct ifname_desc_t *)&ifname_descs[0]; ifd->ifname; ifd++) {
		if (ap_mode && ifd->is_router_mode)
			continue;
		if (!ap_mode && ifd->is_ap_mode)
			continue;
		if (strcmp(ifname, ifd->ifname) == 0) {
			if (wan_no)
				*wan_no = (int)ifd->wan_no;
			if (ifd->is_dynamic)
				*ifindex = get_interface_index(ifname);
			else
				*ifindex = 0;
			return ifd->ifdesc;
		}
	}

	return NULL;
}

int is_usbnet_interface(const char *ifname)
{
	if(!strncmp(ifname, "weth", 4))
		return 1;
	if(!strncmp(ifname, "wwan", 4))
		return 1;
	return 0;
}

static uint64_t
get_ifstats_counter(const char* ifname, const char* cnt_name)
{
	FILE *fp;
	uint64_t cnt_val64 = 0;
	char cnt_path[64], cnt_data[32];

	snprintf(cnt_path, sizeof(cnt_path), "/sys/class/net/%s/statistics/%s", ifname, cnt_name);
	fp = fopen(cnt_path, "r");
	if (fp) {
		if (fgets(cnt_data, sizeof(cnt_data), fp))
			cnt_val64 = strtoull(cnt_data, NULL, 10);
		fclose(fp);
	}

	return cnt_val64;
}

uint64_t
get_ifstats_bytes_rx(const char* ifname)
{
	return get_ifstats_counter(ifname, "rx_bytes");
}

uint64_t
get_ifstats_bytes_tx(const char* ifname)
{
	return get_ifstats_counter(ifname, "tx_bytes");
}

static int
is_invalid_char_for_hostname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
	else if (c >= 0x7b)
		ret = 1;

	return ret;
}

int
is_valid_hostname(const char *hname)
{
	int len, i;

	len = strlen(hname);
	if (len < 1)
		return 0;

	for (i = 0; i < len; i++) {
		if (is_invalid_char_for_hostname(hname[i]))
			return 0;
	}

	return 1;
}

in_addr_t
inet_addr_safe(const char *cp)
{
	struct in_addr a;

	if (!cp)
		return INADDR_ANY;

	if (!inet_aton(cp, &a))
		return INADDR_ANY;
	else
		return a.s_addr;
}

int
is_valid_ipv4(const char *cp)
{
	return (inet_addr_safe(cp) != INADDR_ANY) ? 1 : 0;
}

int
is_man_wisp(const char *ifname)
{
	if (strcmp(ifname, IFNAME_2G_APCLI) == 0
#if BOARD_HAS_5G_RADIO
	 || strcmp(ifname, IFNAME_5G_APCLI) == 0
#endif
	   )
		return 1;
	return 0;
}

int
get_ap_mode(void)
{
	if (nvram_match("wan_route_x", "IP_Bridged"))
		return 1;

	return 0;
}

int
get_wan_proto(int unit)
{
	char tmp[32], prefix[16], *proto;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

	if (strcmp(proto, "pppoe") == 0)
		return IPV4_WAN_PROTO_PPPOE;
	if (strcmp(proto, "pptp") == 0)
		return IPV4_WAN_PROTO_PPTP;
	if (strcmp(proto, "l2tp") == 0)
		return IPV4_WAN_PROTO_L2TP;
	if (strcmp(proto, "dhcp") == 0)
		return IPV4_WAN_PROTO_IPOE_DHCP;
	if (strcmp(proto, "static") == 0)
		return IPV4_WAN_PROTO_IPOE_STATIC;
	return -1;
}

char*
get_man_ifname(int unit)
{
	char tmp[32], prefix[16];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	return nvram_safe_get(strcat_r(prefix, "ifname", tmp));
}

int
get_usb_modem_wan(int unit)
{
	char tmp[32], prefix[16];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (nvram_get_int(strcat_r(prefix, "modem_dev", tmp)) != 0)
		return 1;
	else
		return 0;
}

int
get_usb_modem_dev_wan(int unit, int devnum)
{
	char tmp[32], prefix[16];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (nvram_get_int(strcat_r(prefix, "modem_dev", tmp)) == devnum)
		return 1;
	else
		return 0;
}

void
set_usb_modem_dev_wan(int unit, int devnum)
{
	char tmp[32], prefix[16];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	nvram_set_int_temp(strcat_r(prefix, "modem_dev", tmp), devnum);
}

int
get_wan_ether_link_cached(void)
{
	return (nvram_get_int("link_wan") == 1) ? 1 : 0;
}

int
get_internet_state_cached(void)
{
	return (nvram_get_int("link_internet") == 1) ? 1 : 0;
}

int
ifconfig(const char *ifname, int flags, const char *addr, const char *mask)
{
	int sockfd, ret = 0;
	struct ifreq ifr;
	struct in_addr addr_in, mask_in;

	/* Open a socket to the kernel */
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	/* Set interface name */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	/* Get interface flags */
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		ret = errno;
		goto err;
	}

	/* Set interface flags */
	if (!flags)
		ifr.ifr_flags &= ~(IFF_UP);
	else
		ifr.ifr_flags |= flags;

	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
		ret = errno;
		goto err;
	}

	/* Set IP address */
	if (addr) {
		if (!inet_aton(addr, &addr_in))
			addr_in.s_addr = INADDR_ANY;
		sin_addr(&ifr.ifr_addr).s_addr = addr_in.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
			ret = errno;
			goto err;
		}
	}

	/* Set IP netmask and broadcast */
	if (addr && mask) {
		if (!inet_aton(mask, &mask_in))
			mask_in.s_addr = INADDR_ANY;
		if (addr_in.s_addr != INADDR_ANY && mask_in.s_addr != INADDR_ANY) {
			sin_addr(&ifr.ifr_netmask).s_addr = mask_in.s_addr;
			ifr.ifr_netmask.sa_family = AF_INET;
			if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
				ret = errno;
				goto err;
			}
			
			sin_addr(&ifr.ifr_broadaddr).s_addr = htonl((ntohl(addr_in.s_addr) & ntohl(mask_in.s_addr)) | ~(ntohl(mask_in.s_addr)));
			ifr.ifr_broadaddr.sa_family = AF_INET;
			if (ioctl(sockfd, SIOCSIFBRDADDR, &ifr) < 0) {
				ret = errno;
				goto err;
			}
		}
	}

err:
	close(sockfd);

	return ret;
}

int
is_interface_exist(const char *ifname)
{
	if (get_interface_flags(ifname) < 0)
		return 0;

	return 1;
}

int
is_interface_up(const char *ifname)
{
	int iflags = get_interface_flags(ifname);
	if (iflags < 0)
		return 0;

	if (iflags & IFF_UP)
		return 1;

	return 0;
}

int
get_interface_flags(const char *ifname)
{
	struct ifreq ifr;
	int sockfd, iflags;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
		iflags = -1;
	else
		iflags = ifr.ifr_flags;

	close(sockfd);

	return iflags;
}

int
get_interface_index(const char *ifname)
{
	struct ifreq ifr;
	int sockfd, ifindex;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
		ifindex = -1;
	else
		ifindex = ifr.ifr_ifindex;

	close(sockfd);

	return ifindex;
}

int
get_interface_irq(const char *ifname)
{
	struct ifreq ifr;
	int sockfd, irq;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFMAP, &ifr) < 0)
		irq = -1;
	else
		irq = ifr.ifr_map.irq;

	close(sockfd);

	return irq;
}

int
get_interface_mtu(const char *ifname)
{
	int sockfd;
	struct ifreq ifr;
	int imtu = 1500;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return INADDR_ANY;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	/* Get MTU */
	if (ioctl(sockfd, SIOCGIFMTU, &ifr) == 0)
		imtu = ifr.ifr_mtu;

	close(sockfd);

	return imtu;
}

int
set_interface_mtu(const char *ifname, int mtu)
{
	struct ifreq ifr;
	int sockfd, ret;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_mtu = mtu;

	ret = ioctl(sockfd, SIOCSIFMTU, &ifr);

	close(sockfd);

	return ret;
}

int
get_interface_hwaddr(const char *ifname, unsigned char mac[6])
{
	struct ifreq ifr;
	int sockfd, result = 0;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
		result = -1;
	else
		memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

	close(sockfd);

	return result;
}

int
set_interface_hwaddr(const char *ifname, const char *mac_str)
{
	struct ifreq ifr;
	int sockfd, ret;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	ether_atoe(mac_str, ifr.ifr_hwaddr.sa_data);

	ret = ioctl(sockfd, SIOCSIFHWADDR, &ifr);

	close(sockfd);

	return ret;
}

in_addr_t
get_interface_addr4(const char *ifname)
{
	int sockfd;
	struct ifreq ifr;
	in_addr_t ipv4_addr = INADDR_ANY;

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return INADDR_ANY;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	/* Get IPv4 address */
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
		struct sockaddr_in *ipv4_inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
		if (ipv4_inaddr->sin_addr.s_addr != INADDR_ANY &&
		    ipv4_inaddr->sin_addr.s_addr != INADDR_NONE)
			ipv4_addr = ipv4_inaddr->sin_addr.s_addr;
	}

	close(sockfd);

	return ipv4_addr;
}

void
set_interface_conf_int(const char *ver, const char *ifname, const char *param, int value)
{
	char conf_path[80];

	snprintf(conf_path, sizeof(conf_path), "/proc/sys/net/%s/conf/%s/%s", ver, ifname, param);
	fput_int(conf_path, value);
}

int
vconfig_add_if(const char *ifname, unsigned int vid)
{
	int sockfd, ret;
	struct vlan_ioctl_args ifr;

	if (vid > 4095)
		return -1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.device1, ifname, IFNAMSIZ);
	ifr.cmd = ADD_VLAN_CMD;
	ifr.u.VID = vid;

	ret = ioctl(sockfd, SIOCSIFVLAN, &ifr);

	close(sockfd);

	return ret;
}

int
vconfig_egress_map(const char *vifname, int skb_prio, int qos_prio)
{
	int sockfd, ret;
	struct vlan_ioctl_args ifr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.device1, vifname, sizeof(ifr.device1));
	ifr.cmd = SET_VLAN_EGRESS_PRIORITY_CMD;
	ifr.u.skb_priority = skb_prio;
	ifr.vlan_qos = qos_prio;

	ret = ioctl(sockfd, SIOCSIFVLAN, &ifr);

	close(sockfd);

	return ret;
}

int
vconfig_del_if(const char *vifname)
{
	int sockfd, ret;
	struct vlan_ioctl_args ifr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.device1, vifname, sizeof(ifr.device1));
	ifr.cmd = DEL_VLAN_CMD;

	ret = ioctl(sockfd, SIOCSIFVLAN, &ifr);

	close(sockfd);

	return ret;
}

int
br_add_del_bridge(const char *brname, int is_add)
{
	int sockfd, ret;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	ret = ioctl(sockfd, (is_add) ? SIOCBRADDBR : SIOCBRDELBR, brname);

	close(sockfd);

	return ret;
}

int
br_add_del_if(const char *brname, const char *ifname, int is_add)
{
	int sockfd, ret;
	struct ifreq ifr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
		close(sockfd);
		return -ENODEV;
	}

	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ret = ioctl(sockfd, (is_add) ? SIOCBRADDIF : SIOCBRDELIF, &ifr);

	close(sockfd);

	return ret;
}

int
br_del_all_ifs(const char *brname, int do_if_down, int do_if_rem)
{
#define BR_MAX_IF	32
	struct ifreq ifr;
	int i, sockfd;
	int ifindices[BR_MAX_IF] = {0};
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, BR_MAX_IF, 0 };

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *)&args;
	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr) < 0) {
		close(sockfd);
		return -1;
	}

	ifr.ifr_data = NULL;

	for (i = 0; i < BR_MAX_IF; i++) {
		if (!ifindices[i])
			continue;
		ifr.ifr_ifindex = ifindices[i];
		ioctl(sockfd, SIOCBRDELIF, &ifr);
	}

	if (do_if_down || do_if_rem) {
		for (i = 0; i < BR_MAX_IF; i++) {
			if (!ifindices[i])
				continue;
			memset(&ifr, 0, sizeof(ifr));
			ifr.ifr_ifindex = ifindices[i];
			if (ioctl(sockfd, SIOCGIFNAME, &ifr) < 0)
				continue;
			if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
				continue;
			if (ifr.ifr_flags & IFF_UP) {
				ifr.ifr_flags &= ~(IFF_UP);
				if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
					continue;
			}
			if (do_if_rem && strcmp(ifr.ifr_name, IFNAME_LAN))
				vconfig_del_if(ifr.ifr_name);
		}
	}

	close(sockfd);

	return 0;
}

void
br_set_param_int(const char *brname, const char *param, int value)
{
	char bridge_path[80];

	snprintf(bridge_path, sizeof(bridge_path), "/sys/class/net/%s/bridge/%s", brname, param);
	fput_int(bridge_path, value);
}

void
br_set_stp(const char *brname, int stp_state)
{
	br_set_param_int(brname, "stp_state", stp_state);
}

void
br_set_fd(const char *brname, int delay_sec)
{
	br_set_param_int(brname, "forward_delay", delay_sec * 100);
}

void
brport_set_param_int(const char *ifname, const char *param, int value)
{
	char brport_path[80];

	snprintf(brport_path, sizeof(brport_path), "/sys/class/net/%s/brport/%s", ifname, param);
	fput_int(brport_path, value);
}

void
brport_set_m2u(const char *ifname, int m2u_on)
{
	int i_value = (m2u_on) ? 1 : 0;

	brport_set_param_int(ifname, "multicast_fast_leave", i_value);
	brport_set_param_int(ifname, "multicast_to_unicast", i_value);
}

int get_ipv6_type(void)
{
#if defined (USE_IPV6)
	int i;
	const char *ipv6_svc_type;
	const char *ipv6_svc_names[] = {
		"static",	// IPV6_NATIVE_STATIC
		"dhcp6",	// IPV6_NATIVE_DHCP
		"6in4",		// IPV6_6IN4
		"6to4",		// IPV6_6TO4
		"6rd",		// IPV6_6RD
		NULL
	};

	ipv6_svc_type = nvram_safe_get("ip6_service");
	if (!(*ipv6_svc_type))
		return IPV6_DISABLED;
	
	for (i = 0; ipv6_svc_names[i] != NULL; i++) {
		if (strcmp(ipv6_svc_type, ipv6_svc_names[i]) == 0) return i + 1;
	}
#endif
	return IPV6_DISABLED;
}

#if defined (USE_IPV6)
int is_valid_ipv6(const char *cp)
{
	struct in6_addr a6;

	if (!cp)
		return 0;

	if (inet_pton(AF_INET6, cp, &a6) != 1)
		return 0;

	return 1;
}

static int get_prefix6_len(struct sockaddr_in6 *mask6)
{
	int i, j, prefix = 0;
	unsigned char *netmask = (unsigned char *) &(mask6)->sin6_addr;

	for (i = 0; i < 16; i++, prefix += 8)
		if (netmask[i] != 0xff)
			break;

	if (i != 16 && netmask[i])
		for (j = 7; j > 0; j--, prefix++)
			if ((netmask[i] & (1 << j)) == 0)
				break;

	return prefix;
}

char *get_ifaddr6(const char *ifname, int linklocal, char *p_addr6s)
#if defined (HAVE_GETIFADDRS)
{
	char *ret = NULL;
	int prefix;
	struct ifaddrs *ifap, *ife;
	const struct sockaddr_in6 *addr6;

	if (getifaddrs(&ifap) < 0)
		return NULL;

	for (ife = ifap; ife; ife = ife->ifa_next)
	{
		if (strcmp(ifname, ife->ifa_name) != 0)
			continue;
		if (ife->ifa_addr == NULL)
			continue;
		if (ife->ifa_addr->sa_family == AF_INET6)
		{
			addr6 = (const struct sockaddr_in6 *)ife->ifa_addr;
			if (IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr) ^ linklocal)
				continue;
			if (inet_ntop(ife->ifa_addr->sa_family, &addr6->sin6_addr, p_addr6s, INET6_ADDRSTRLEN) != NULL) {
				prefix = get_prefix6_len((struct sockaddr_in6 *)ife->ifa_netmask);
				if (prefix > 0 && prefix < 128)
					sprintf(p_addr6s, "%s/%d", p_addr6s, prefix);
				ret = p_addr6s;
				break;
			}
		}
	}
	freeifaddrs(ifap);
	return ret;
}
#else
/* getifaddrs replacement */
{
	FILE *fp;
	char *ret = NULL;
	char addr6s[INET6_ADDRSTRLEN], addr6p[8][8], devname[32];
	int if_idx, plen, scope, scope_need, dad_status;
	struct in6_addr addr6;

	scope_need = (linklocal) ? 0x20 : 0x00;

	fp = fopen("/proc/net/if_inet6", "r");
	if (!fp)
		return NULL;
	while (fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
		addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
		addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
		&dad_status, devname) != EOF)
	{
		if (strcmp(ifname, devname) != 0)
			continue;
		scope = scope & 0x00f0;
		if (scope != scope_need)
			continue;
		sprintf(addr6s, "%s:%s:%s:%s:%s:%s:%s:%s",
			addr6p[0], addr6p[1], addr6p[2], addr6p[3],
			addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
		if (inet_pton(AF_INET6, addr6s, &addr6) > 0 &&
		    inet_ntop(AF_INET6, &addr6, p_addr6s, INET6_ADDRSTRLEN) != NULL) {
			if (plen > 0 && plen < 128)
				sprintf(p_addr6s, "%s/%d", p_addr6s, plen);
			ret = p_addr6s;
			break;
		}
	}
	fclose(fp);
	return ret;
}
#endif
#endif


