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

#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ABS
#define	ABS(a)			(((a) < 0)?-(a):(a))
#endif /* ABS */

#ifndef MIN
#define	MIN(a, b)		(((a) < (b))?(a):(b))
#endif /* MIN */

#ifndef MAX
#define	MAX(a, b)		(((a) > (b))?(a):(b))
#endif /* MAX */


enum{
	RESTART_HTTPD		= 0x00000001,
	RESTART_DNS		= 0x00000002,
	RESTART_DDNS		= 0x00000004,
	RESTART_UPNP		= 0x00000008,

	RESTART_FIREWALL	= 0x00000010,
	RESTART_DHCPD		= 0x00000020,
	RESTART_RADVD		= 0x00000040,
	RESTART_IPTV		= 0x00000080,

	RESTART_TIME		= 0x00000100,
	RESTART_NTPC		= 0x00000200,
	RESTART_SYSLOG		= 0x00000400,
	RESTART_WDG_CPU		= 0x00000800,

	RESTART_TERMINAL	= 0x00001000,
	RESTART_VPNSVR		= 0x00002000,
	RESTART_VPNCLI		= 0x00004000,
	RESTART_SYSCTL		= 0x00008000,

	RESTART_SPOOLER		= 0x00010000,
	RESTART_HDDTUNE		= 0x00020000,
	RESTART_FTPSAMBA	= 0x00040000,
	RESTART_NFS		= 0x00080000,

	RESTART_DMS		= 0x00100000,
	RESTART_ITUNES		= 0x00200000,
	RESTART_TORRENT		= 0x00400000,
	RESTART_ARIA		= 0x00800000,

	RESTART_WIFI		= 0x01000000,
	RESTART_SWITCH		= 0x02000000,
	RESTART_SWITCH_VLAN	= 0x04000000,
	RESTART_MODEM		= 0x08000000,

	RESTART_LAN		= 0x10000000,
	RESTART_WAN		= 0x20000000,
	RESTART_IPV6		= 0x40000000,
	RESTART_REBOOT		= 0x80000000
};

#define ITVL_RESTART_HTTPD		2
#define ITVL_RESTART_DNS		1
#define ITVL_RESTART_DDNS		1
#define ITVL_RESTART_UPNP		1

#define ITVL_RESTART_FIREWALL		1
#define ITVL_RESTART_DHCPD		1
#define ITVL_RESTART_RADVD		1
#define ITVL_RESTART_IPTV		1

#define ITVL_RESTART_TIME		2
#define ITVL_RESTART_NTPC		1
#define ITVL_RESTART_SYSLOG		1
#define ITVL_RESTART_WDG_CPU		1

#define ITVL_RESTART_TERMINAL		1
#define ITVL_RESTART_VPNSVR		2
#define ITVL_RESTART_VPNCLI		2
#define ITVL_RESTART_SYSCTL		1

#define ITVL_RESTART_SPOOLER		1
#define ITVL_RESTART_HDDTUNE		1
#define ITVL_RESTART_FTPSAMBA		2
#define ITVL_RESTART_NFS		2

#define ITVL_RESTART_DMS		2
#define ITVL_RESTART_ITUNES		2
#define ITVL_RESTART_TORRENT		3
#define ITVL_RESTART_ARIA		3

#define ITVL_RESTART_WIFI		3
#if defined(USE_RT3352_MII)
 #define ITVL_RESTART_WIFI_INIC		5
#else
 #define ITVL_RESTART_WIFI_INIC		3
#endif
#define ITVL_RESTART_SWITCH		3
#define ITVL_RESTART_SWITCH_VLAN	3
#define ITVL_RESTART_MODEM		5

#define ITVL_RESTART_WAN		5
#define ITVL_RESTART_LAN		3
#define ITVL_RESTART_REBOOT		40


struct variable {
	char *name;
	char *longname;
	char **argv;
	int event;
};

struct svcLink
{
      char *serviceId;
      char *serviceType;
      struct variable *variables;
};

#define ARGV(args...) ((char *[]) { args, NULL })

/* API export for UPnP function */
int LookupServiceId(char *serviceId);
char *GetServiceId(int sid);
struct variable *GetVariables(int sid);


#endif /* _COMMON_H_ */
