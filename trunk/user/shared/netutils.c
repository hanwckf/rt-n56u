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
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "nvram/bcmnvram.h"
#include "netutils.h"
#include "shutils.h"

#if defined (HAVE_GETIFADDRS)
#include <ifaddrs.h>
#endif

static const struct ifname_desc_t {
	const char *ifname;
	const char *ifdesc;
} ifname_descs[] = {
	{ IFNAME_LAN,     IFDESC_LAN      },
	{ IFNAME_2G_MAIN, IFDESC_WLAN_2G  },
#if BOARD_HAS_5G_RADIO
	{ IFNAME_5G_MAIN, IFDESC_WLAN_5G  },
#endif
	{ IFNAME_WAN,     IFDESC_WAN      },
	{ IFNAME_RAS,     IFDESC_WWAN     },
	{ IFNAME_USBNET1, IFDESC_WWAN     },
	{ IFNAME_USBNET2, IFDESC_WWAN     },
	{ NULL, NULL }
};

const char* get_ifname_descriptor(const char* ifname)
{
	struct ifname_desc_t *ifd;

	for (ifd = (struct ifname_desc_t *)&ifname_descs[0]; ifd->ifname; ifd++) {
		if (strcmp(ifname, ifd->ifname) == 0)
			return ifd->ifdesc;
	}

	return NULL;
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
ifconfig(char *ifname, int flags, char *addr, char *mask)
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

char *get_ifaddr6(char *ifname, int linklocal, char *p_addr6s)
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


