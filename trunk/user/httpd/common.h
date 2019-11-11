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

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

#define EVM_RESTART_FIREWALL		(1ULL <<  0)
#define EVM_RESTART_DHCPD		(1ULL <<  1)
#define EVM_RESTART_RADV		(1ULL <<  2)
#define EVM_RESTART_DDNS		(1ULL <<  3)
#define EVM_RESTART_UPNP		(1ULL <<  4)
#define EVM_RESTART_TIME		(1ULL <<  5)
#define EVM_RESTART_NTPC		(1ULL <<  6)
#define EVM_RESTART_SYSLOG		(1ULL <<  7)
#define EVM_RESTART_NETFILTER		(1ULL <<  8)
#define EVM_REAPPLY_VPNSVR		(1ULL <<  9)
#define EVM_RESTART_VPNSVR		(1ULL << 10)
#define EVM_RESTART_VPNCLI		(1ULL << 11)
#define EVM_RESTART_WIFI2		(1ULL << 12)
#define EVM_RESTART_WIFI5		(1ULL << 13)
#define EVM_RESTART_SWITCH_CFG		(1ULL << 14)
#define EVM_RESTART_SWITCH_VLAN		(1ULL << 15)
#define EVM_RESTART_LAN			(1ULL << 17)
#define EVM_RESTART_WAN			(1ULL << 18)
#define EVM_RESTART_IPV6		(1ULL << 19)
#define EVM_RESTART_HTTPD		(1ULL << 20)
#define EVM_RESTART_TELNETD		(1ULL << 21)
#define EVM_RESTART_SSHD		(1ULL << 22)
#define EVM_RESTART_WINS		(1ULL << 23)
#define EVM_RESTART_LLTD		(1ULL << 24)
#define EVM_RESTART_ADSC		(1ULL << 25)
#define EVM_RESTART_IPTV		(1ULL << 26)
#define EVM_RESTART_CROND		(1ULL << 27)
#define EVM_RESTART_SYSCTL		(1ULL << 28)
#define EVM_RESTART_TWEAKS		(1ULL << 29)
#define EVM_RESTART_WDG			(1ULL << 30)
#define EVM_RESTART_DI			(1ULL << 31)
#define EVM_RESTART_SPOOLER		(1ULL << 32)
#define EVM_RESTART_MODEM		(1ULL << 33)
#define EVM_RESTART_HDDTUNE		(1ULL << 34)
#define EVM_RESTART_FTPD		(1ULL << 35)
#define EVM_RESTART_NMBD		(1ULL << 36)
#define EVM_RESTART_SMBD		(1ULL << 37)
#define EVM_RESTART_NFSD		(1ULL << 38)
#define EVM_RESTART_DMS			(1ULL << 39)
#define EVM_RESTART_ITUNES		(1ULL << 40)
#define EVM_RESTART_TRMD		(1ULL << 41)
#define EVM_RESTART_ARIA		(1ULL << 42)
#define EVM_RESTART_SCUT		(1ULL << 43)
#define EVM_RESTART_TTYD		(1ULL << 44)
#define EVM_RESTART_VLMCSD		(1ULL << 45)
#define EVM_RESTART_DNSFORWARDER	(1ULL << 47)
#define EVM_RESTART_SHADOWSOCKS	(1ULL << 48)
#define EVM_RESTART_SS_TUNNEL		(1ULL << 49)
#define EVM_RESTART_MENTOHUST		(1ULL << 50)

#define EVM_RESTART_REBOOT		(1ULL << 62)

#define EVM_BLOCK_UNSAFE		(1ULL << 63) /* special case */


#define EVT_RESTART_FIREWALL		1
#define EVT_RESTART_DHCPD		1
#define EVT_RESTART_RADV		1
#define EVT_RESTART_DDNS		1
#define EVT_RESTART_UPNP		1
#define EVT_RESTART_TIME		2
#define EVT_RESTART_NTPC		1
#define EVT_RESTART_SYSLOG		1
#define EVT_RESTART_NETFILTER		1
#define EVT_REAPPLY_VPNSVR		1
#define EVT_RESTART_VPNSVR		2
#define EVT_RESTART_VPNCLI		2
#if defined (USE_RT3352_MII)
#define EVT_RESTART_WIFI2		5
#else
#define EVT_RESTART_WIFI2		3
#endif
#define EVT_RESTART_WIFI5		3
#define EVT_RESTART_SWITCH_CFG		3
#define EVT_RESTART_SWITCH_VLAN		3
#define EVT_RESTART_LAN			5
#define EVT_RESTART_WAN			5
#define EVT_RESTART_IPV6		3
#define EVT_RESTART_HTTPD		2
#define EVT_RESTART_TELNETD		1
#define EVT_RESTART_SSHD		1
#define EVT_RESTART_WINS		2
#define EVT_RESTART_LLTD		1
#define EVT_RESTART_ADSC		1
#define EVT_RESTART_CROND		1
#define EVT_RESTART_IPTV		1
#define EVT_RESTART_SYSCTL		1
#define EVT_RESTART_TWEAKS		1
#define EVT_RESTART_WDG			1
#define EVT_RESTART_DI			1
#define EVT_RESTART_SPOOLER		1
#define EVT_RESTART_MODEM		3
#define EVT_RESTART_HDDTUNE		1
#define EVT_RESTART_FTPD		1
#define EVT_RESTART_NMBD		2
#define EVT_RESTART_SMBD		2
#define EVT_RESTART_NFSD		2
#define EVT_RESTART_DMS			2
#define EVT_RESTART_ITUNES		2
#define EVT_RESTART_TRMD		3
#define EVT_RESTART_ARIA		3
#define EVT_RESTART_SCUT		1
#define EVT_RESTART_TTYD		1
#define EVT_RESTART_VLMCSD		1
#define EVT_RESTART_SHADOWSOCKS	2
#define EVT_RESTART_SS_TUNNEL		2
#define EVT_RESTART_DNSFORWARDER	1
#define EVT_RESTART_MENTOHUST		2
#define EVT_RESTART_REBOOT		40

struct variable
{
	const char *name;
	const char *longname;
	char **argv;
	u64 event_mask;
};

struct svcLink
{
	const char *serviceId;
	struct variable *variables;
};

struct evDesc
{
	u64 event_mask;
	u32 max_time;
	const char* notify_cmd;
	u64 event_unmask;
};

#define ARGV(args...) ((char *[]) { args, NULL })

/* API export for UPnP function */
int LookupServiceId(char *serviceId);
const char *GetServiceId(int sid);
struct variable *GetVariables(int sid);


#endif /* _COMMON_H_ */
