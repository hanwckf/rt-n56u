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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#include <iwlib.h>
#include <ralink_priv.h>

#include "common.h"
#include "httpd.h"

/******************************************************************************************************************************************/

/* Dump leases in <tr><td>hostname</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
int
ej_lan_leases(int eid, webs_t wp, int argc, char **argv)
{
	/* dnsmasq ex: 43200 00:26:18:57:08:bc 192.168.1.105 mypc-3eaf6880a0 01:00:26:18:57:08:bc */
	
	FILE *fp = NULL;
	int ret = 0;
	int i;
	char buff[256], dh_lease[32], dh_mac[64], dh_ip[64], dh_host[64];
#if defined (USE_IPV6)
	int ip6_count = 0;
	struct in6_addr addr6;
#endif

	ret += websWrite(wp, "IPv4 Address       MAC Address          Host Name\n");
	//                    192.168.100.100    00:90:F5:XX:XX:XX    Padavan
	//                    ffff:ffff:ffff:0001:0000:0000:0000:0001 Padavan

	if (!(fp = fopen("/tmp/dnsmasq.leases", "r")))
		return ret;

	while (fgets(buff, sizeof(buff), fp)) {
		if (sscanf(buff, "%s %s %s %s %*s", dh_lease, dh_mac, dh_ip, dh_host) != 4)
			continue;
		
		if (strcmp(dh_lease, "duid") == 0)
			continue;
		
#if defined (USE_IPV6)
		if (inet_pton(AF_INET6, dh_ip, &addr6) != 0) {
			ip6_count++;
			continue;
		}
#endif
		strcat(dh_lease, " secs");
		
		if (!dh_host[0])
			strcpy(dh_host, "*");
		
		// convert MAC to upper case
		for (i=0; i<strlen(dh_mac); i++)
			dh_mac[i] = toupper(dh_mac[i]);
		
		ret += websWrite(wp, "%-19s", (dh_ip[0]!=0) ? dh_ip : " ");
		ret += websWrite(wp, "%-21s", (dh_mac[0]!=0) ? dh_mac : " " );
		ret += websWrite(wp, "%s\n",  dh_host);
	}
	fclose(fp);

#if defined (USE_IPV6)
	if (ip6_count < 1)
		return ret;

	ret += websWrite(wp, "\nIPv6 Address                            Host Name\n");
	//                      ffff:ffff:ffff:0001:0000:0000:0000:0001 Padavan

	fp = fopen("/tmp/dnsmasq.leases", "r");
	if (fp) {
		while (fgets(buff, sizeof(buff), fp)) {
			if (sscanf(buff, "%s %s %s %s %*s", dh_lease, dh_mac, dh_ip, dh_host) != 4)
				continue;
			
			if (strcmp(dh_lease, "duid") == 0)
				continue;
			
			if (inet_pton(AF_INET6, dh_ip, &addr6) == 0)
				continue;
			
			strcat(dh_lease, " secs");
			
			if (!dh_host[0])
				strcpy(dh_host, "*");
			
			ret += websWrite(wp, "%-40s", (dh_ip[0]!=0) ? dh_ip : " ");
			ret += websWrite(wp, "%s\n",  dh_host);
		}
		fclose(fp);
	}
#endif

	return ret;
}

int
ej_vpns_leases(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	int ret = 0, i_clients = 0;
	char ifname[16], addr_l[32], addr_r[32], peer_name[64];
	
	ret += websWrite(wp, "#  IP Local         IP Remote        Login          NetIf\n");
	
	if (!(fp = fopen("/tmp/vpns.leases", "r"))) {
		return ret;
	}
	
	while (fscanf(fp, "%s %s %s %[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4) 
	{
		i_clients++;
		ret += websWrite(wp, "%-3u", i_clients);
		ret += websWrite(wp, "%-17s", addr_l);
		ret += websWrite(wp, "%-17s", addr_r);
		ret += websWrite(wp, "%-15s", peer_name);
		ret += websWrite(wp, "%s\n",  ifname);
	}
	fclose(fp);
	
	return ret;
}

int is_hwnat_loaded()
{
	DIR *dir_to_open = NULL;
	FILE *fp;
	char offload_val[32];
	
	dir_to_open = opendir("/sys/module/hw_nat");
	if (dir_to_open)
	{
		closedir(dir_to_open);
		fp = fopen("/sys/module/hw_nat/parameters/wifi_offload", "r");
		if (fp) {
			offload_val[0] = 0;
			fgets(offload_val, sizeof(offload_val), fp);
			fclose(fp);
			if (strlen(offload_val) > 0)
				offload_val[strlen(offload_val) - 1] = 0; /* get rid of '\n' */
			
			if (offload_val[0] == 'Y' || offload_val[0] == '1')
				return 2;
		}
		
		return 1;
	}
	
	return 0;
}

/* Dump NAT table <tr><td>destination</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
int
ej_nat_table(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	int ret, i_loaded, sw_mode;
	char line[256], tmp[255], target[16], proto[16], src[16], dst[16];
	char *range, *host, *port, *ptr, *val;
	char *hwnat_status;
	char *nat_argv[] = {"iptables", "-t", "nat", "-nxL", NULL};
	
	ret = 0;
	sw_mode = nvram_get_int("sw_mode");
	if (sw_mode == 1 || sw_mode == 4)
	{
		hwnat_status = "Disabled";
		
		i_loaded = is_hwnat_loaded();
		if (i_loaded == 2)
#if defined(USE_WWAN_HW_NAT)
			hwnat_status = "Enabled, IPoE/PPPoE offload [WAN/WWAN]<->[LAN/WLAN]";
#else
			hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/WLAN]";
#endif
		else if (i_loaded == 1)
			hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN]";
		
		ret += websWrite(wp, "Hardware NAT/Routing: %s\n", hwnat_status);
	}
	
	if (sw_mode == 1)
	{
//		ret += websWrite(wp, "Software QoS: %s\n", nvram_match("qos_enable", "1") ? "Enabled": "Disabled");
		ret += websWrite(wp, "\n");
		
		ret += websWrite(wp, "Port Forwards List\n");
		ret += websWrite(wp, "----------------------------------------\n");
		ret += websWrite(wp, "Destination     Proto. Port Range  Redirect to     Local port\n");
//                                    255.255.255.255 other  65535:65535 255.255.255.255 65535:65535

		_eval(nat_argv, ">/tmp/nat.log", 3, NULL);

		fp = fopen("/tmp/nat.log", "r");
		if (fp == NULL)
			return ret;

		while (fgets(line, sizeof(line), fp) != NULL)
		{
			tmp[0] = 0;
			if (sscanf(line,
			    "%15s%*[ \t]"		// target
			    "%15s%*[ \t]"		// prot
			    "%*s%*[ \t]"		// opt
			    "%15[^/]/%*d%*[ \t]"	// source
			    "%15[^/]/%*d%*[ \t]"	// destination
			    "%255[^\n]",		// options
			    target, proto, src, dst, tmp) < 5) continue;
			
			if (strcmp(target, "DNAT"))
				continue;
			
			for (ptr = proto; *ptr; ptr++)
				*ptr = toupper(*ptr);
			
			if (!strcmp(dst, "0.0.0.0"))
				strcpy(dst, "ALL");
			
			port = host = range = "";
			ptr = tmp;
			while ((val = strsep(&ptr, " ")) != NULL) {
				if (strncmp(val, "dpt:", 4) == 0)
					range = val + 4;
				else if (strncmp(val, "dpts:", 5) == 0)
					range = val + 5;
				else if (strncmp(val, "to:", 3) == 0) {
					port = host = val + 3;
					strsep(&port, ":");
				}
			}
			
			ret += websWrite(wp,
				"%-15s %-6s %-11s %-15s %-11s\n",
				dst, proto, range, host, port ? : range);
		}
		fclose(fp);
	}

	return ret;
}

int
ej_route_table(int eid, webs_t wp, int argc, char **argv)
{
	char buff[256];
	int  nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int flgs, ref, use, metric, ret;
	char flags[4];
	unsigned long int d,g,m;
	char sdest[16], sgw[16];
	FILE *fp;

	ret = 0;
	ret += websWrite(wp, "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface\n");

	if (!(fp = fopen("/proc/net/route", "r"))) return 0;

	while (fgets(buff, sizeof(buff), fp) != NULL ) 
	{
		if (nl) 
		{
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if (sscanf(buff+ifl+1, "%lx%lx%d%d%d%d%lx",
			   &d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				//error_msg_and_die( "Unsuported kernel route format\n");
				//continue;
			}
			
			ifl = 0;	/* parse flags */
			if (flgs&1)
				flags[ifl++]='U';
			if (flgs&2)
				flags[ifl++]='G';
			if (flgs&4)
				flags[ifl++]='H';
			flags[ifl]=0;
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			strcpy(sdest,  (dest.s_addr==0 ? "default" :
					inet_ntoa(dest)));
			strcpy(sgw,    (gw.s_addr==0   ? "*"       :
					inet_ntoa(gw)));
			
			ret += websWrite(wp, "%-16s%-16s%-16s%-6s%-6d %-2d %7d %s\n",
				sdest, sgw, inet_ntoa(mask), flags, metric, ref, use, buff);
		}
		nl++;
	}
	fclose(fp);

	return ret;
}

int 
ej_conntrack_table(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	int ret;
	char buff[1024], ipv[16], proto[16], state[32], src[64], dst[64], sport[8], dport[8];

	ret = 0;
	ret += websWrite(wp, "Proto   Source Address & Port                           Destination Address & Port\n");
	//                    tcp     222.222.222.222:65535                           222.222.222.222:65535
	//                    tcp     [ffff:ffff:ffff:0001:0000:0000:0000:0001]:65535 [ffff:ffff:ffff:0001:0000:0000:0000:0001]:65535

	if (!(fp = fopen("/proc/net/nf_conntrack", "r"))) return 0;

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%s %*s %s", ipv, proto) < 2)
			continue;
		
		if (strcmp(proto, "tcp") == 0 || strcmp(proto, "sctp") == 0) {
			if (sscanf(buff, "%*s %*s %*s %*s %*s %s src=%s dst=%s sport=%s dport=%s", state, src, dst, sport, dport) < 5)
				continue;
			if (strcmp(state, "ESTABLISHED") != 0)
				continue;
		} else {
			if (sscanf(buff, "%*s %*s %*s %*s %*s src=%s dst=%s sport=%s dport=%s", src, dst, sport, dport) < 4)
				continue;
		}
#if defined (USE_IPV6)
		if (strcmp(ipv, "ipv6") == 0) {
			struct in6_addr addr6;
			if (inet_pton(AF_INET6, src, &addr6) == 1) {
				inet_ntop(AF_INET6, &addr6, src+1, sizeof(src)-1);
				src[0] = '[';
				strcat(src, "]");
			}
			if (inet_pton(AF_INET6, dst, &addr6) == 1) {
				inet_ntop(AF_INET6, &addr6, dst+1, sizeof(dst)-1);
				dst[0] = '[';
				strcat(dst, "]");
			}
		}
#endif
		strcat(src, ":");
		strcat(src, sport);
		strcat(dst, ":");
		strcat(dst, dport);
		ret += websWrite(wp, "%-8s%-48s%s\n", proto, src, dst);
	}

	fclose(fp);

	return ret;
}


/************************ CONSTANTS & MACROS ************************/

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/****************************** TYPES ******************************/

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union	iw_range_raw
{
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/* Disable runtime version warning in ralink_get_range_info() */
int	iw_ignore_version_sp = 0;

/*------------------------------------------------------------------*/
/*
 * Get the range information out of the driver
 */
int
ralink_get_range_info(iwrange *	range, char* buffer, int length)
{
  union iw_range_raw *	range_raw;

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  /* For new versions, we can check the version directly, for old versions
   * we use magic. 300 bytes is a also magic number, don't touch... */
  if (length < 300)
    {
      /* That's v10 or earlier. Ouch ! Let's make a guess...*/
      range_raw->range.we_version_compiled = 9;
    }

  /* Check how it needs to be processed */
  if (range_raw->range.we_version_compiled > 15)
    {
      /* This is our native format, that's easy... */
      /* Copy stuff at the right place, ignore extra */
      memcpy((char *) range, buffer, sizeof(iwrange));
    }
  else
    {
      /* Zero unknown fields */
      bzero((char *) range, sizeof(struct iw_range));

      /* Initial part unmoved */
      memcpy((char *) range,
	     buffer,
	     iwr15_off(num_channels));
      /* Frequencies pushed futher down towards the end */
      memcpy((char *) range + iwr_off(num_channels),
	     buffer + iwr15_off(num_channels),
	     iwr15_off(sensitivity) - iwr15_off(num_channels));
      /* This one moved up */
      memcpy((char *) range + iwr_off(sensitivity),
	     buffer + iwr15_off(sensitivity),
	     iwr15_off(num_bitrates) - iwr15_off(sensitivity));
      /* This one goes after avg_qual */
      memcpy((char *) range + iwr_off(num_bitrates),
	     buffer + iwr15_off(num_bitrates),
	     iwr15_off(min_rts) - iwr15_off(num_bitrates));
      /* Number of bitrates has changed, put it after */
      memcpy((char *) range + iwr_off(min_rts),
	     buffer + iwr15_off(min_rts),
	     iwr15_off(txpower_capa) - iwr15_off(min_rts));
      /* Added encoding_login_index, put it after */
      memcpy((char *) range + iwr_off(txpower_capa),
	     buffer + iwr15_off(txpower_capa),
	     iwr15_off(txpower) - iwr15_off(txpower_capa));
      /* Hum... That's an unexpected glitch. Bummer. */
      memcpy((char *) range + iwr_off(txpower),
	     buffer + iwr15_off(txpower),
	     iwr15_off(avg_qual) - iwr15_off(txpower));
      /* Avg qual moved up next to max_qual */
      memcpy((char *) range + iwr_off(avg_qual),
	     buffer + iwr15_off(avg_qual),
	     sizeof(struct iw_quality));
    }

  /* We are now checking much less than we used to do, because we can
   * accomodate more WE version. But, there are still cases where things
   * will break... */
  if (!iw_ignore_version_sp)
    {
      /* We don't like very old version (unfortunately kernel 2.2.X) */
      if (range->we_version_compiled <= 10)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with an ancient version\n", "raxx");
	  fprintf(stderr, "of Wireless Extension, while this program support version 11 and later.\n");
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* We don't like future versions of WE, because we can't cope with
       * the unknown */
      if (range->we_version_compiled > WE_MAX_VERSION)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with version %d\n", "raxx", range->we_version_compiled);
	  fprintf(stderr, "of Wireless Extension, while this program supports up to version %d.\n", WE_VERSION);
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* Driver version verification */
      if ((range->we_version_compiled > 10) &&
	 (range->we_version_compiled < range->we_version_source))
	{
	  fprintf(stderr, "Warning: Driver for device %s recommend version %d of Wireless Extension,\n", "raxx", range->we_version_source);
	  fprintf(stderr, "but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
	  fprintf(stderr, "may not be available...\n\n");
	}
      /* Note : we are only trying to catch compile difference, not source.
       * If the driver source has not been updated to the latest, it doesn't
       * matter because the new fields are set to zero */
    }

  /* Don't complain twice.
   * In theory, the test apply to each individual driver, but usually
   * all drivers are compiled from the same kernel. */
  iw_ignore_version_sp = 1;

  return (0);
}

#define RTPRIV_IOCTL_SHOW			(SIOCIWFIRSTPRIV + 0x11)
#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)

int
wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
 	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

char* GetBW(int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";
		case BW_20:
			return "20M";
		case BW_40:
			return "40M";
		case BW_80:
			return "80M";
		default:
			return "N/A";
	}
}

char* GetPhyMode(int Mode)
{
	switch(Mode)
	{
		case MODE_CCK:
			return "CCK";
		case MODE_OFDM:
			return "OFDM";
		case MODE_HTMIX:
			return "HTMIX";
		case MODE_HTGREENFIELD:
			return "GREEN";
		case MODE_VHT:
			return "VHT";
		default:
			return "N/A";
	}
}

int MCSMappingRateTable[] =
{
	 2,  4,   11,  22,								// CCK

	12,  18,  24,  36,  48,  72,  96, 108,						// OFDM

	13,  26,  39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,	// 11n: 20MHz, 800ns GI, MCS: 0 ~ 15
	39,  78, 117, 156, 234, 312, 351, 390,						// 11n: 20MHz, 800ns GI, MCS: 16 ~ 23
	27,  54,  81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,	// 11n: 40MHz, 800ns GI, MCS: 0 ~ 15
	81, 162, 243, 324, 486, 648, 729, 810,						// 11n: 40MHz, 800ns GI, MCS: 16 ~ 23
	14,  29,  43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,	// 11n: 20MHz, 400ns GI, MCS: 0 ~ 15
	43,  87, 130, 173, 260, 317, 390, 433,						// 11n: 20MHz, 400ns GI, MCS: 16 ~ 23
	30,  60,  90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,	// 11n: 40MHz, 400ns GI, MCS: 0 ~ 15
	90, 180, 270, 360, 540, 720, 810, 900,

	13,  26,  39,  52,  78, 104, 117, 130, 156,					// 11ac: 20Mhz, 800ns GI, MCS: 0~8
	27,  54,  81, 108, 162, 216, 243, 270, 324, 360,				// 11ac: 40Mhz, 800ns GI, MCS: 0~9
	59, 117, 176, 234, 351, 468, 527, 585, 702, 780,				// 11ac: 80Mhz, 800ns GI, MCS: 0~9
	14,  29,  43,  57,  87, 115, 130, 144, 173,					// 11ac: 20Mhz, 400ns GI, MCS: 0~8
	30,  60,  90, 120, 180, 240, 270, 300, 360, 400,				// 11ac: 40Mhz, 400ns GI, MCS: 0~9
	65, 130, 195, 260, 390, 520, 585, 650, 780, 867					// 11ac: 80Mhz, 400ns GI, MCS: 0~9
};

int
getRate(MACHTTRANSMIT_SETTING HTSetting)
{
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;

	if (HTSetting.field.MODE >= MODE_VHT) {
		if (HTSetting.field.BW == BW_20)
			rate_index = 108 + ((unsigned char)HTSetting.field.ShortGI * 29) + ((unsigned char)HTSetting.field.MCS);
		else if (HTSetting.field.BW == BW_40)
			rate_index = 117 + ((unsigned char)HTSetting.field.ShortGI * 29) + ((unsigned char)HTSetting.field.MCS);
		else if (HTSetting.field.BW == BW_80)
			rate_index = 127 + ((unsigned char)HTSetting.field.ShortGI * 29) + ((unsigned char)HTSetting.field.MCS);
	}
	else if (HTSetting.field.MODE >= MODE_HTMIX)
		rate_index = 12 + ((unsigned char)HTSetting.field.BW * 24) + ((unsigned char)HTSetting.field.ShortGI * 48) + ((unsigned char)HTSetting.field.MCS);
	else if (HTSetting.field.MODE == MODE_OFDM)
		rate_index = (unsigned char)(HTSetting.field.MCS) + 4;
	else if (HTSetting.field.MODE == MODE_CCK)
		rate_index = (unsigned char)(HTSetting.field.MCS);

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count-1;

	return (MCSMappingRateTable[rate_index] * 5)/10;
}

int
getRate_2g(MACHTTRANSMIT_SETTING_2G HTSetting)
{
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;

	if (HTSetting.field.MODE >= MODE_HTMIX)
		rate_index = 12 + ((unsigned char)HTSetting.field.BW * 24) + ((unsigned char)HTSetting.field.ShortGI * 48) + ((unsigned char)HTSetting.field.MCS);
	else if (HTSetting.field.MODE == MODE_OFDM)
		rate_index = (unsigned char)(HTSetting.field.MCS) + 4;
	else if (HTSetting.field.MODE == MODE_CCK)
		rate_index = (unsigned char)(HTSetting.field.MCS);

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count-1;

	return (MCSMappingRateTable[rate_index] * 5)/10;
}

int
get_apcli_peer_connected(const char *ifname, struct iwreq *p_wrq)
{
	int apcli_con = 0;

	if (wl_ioctl(ifname, SIOCGIWAP, p_wrq) >= 0)
	{
		p_wrq->u.ap_addr.sa_family = ARPHRD_ETHER;
		if (p_wrq->u.ap_addr.sa_data[0] ||
		    p_wrq->u.ap_addr.sa_data[1] ||
		    p_wrq->u.ap_addr.sa_data[2] ||
		    p_wrq->u.ap_addr.sa_data[3] ||
		    p_wrq->u.ap_addr.sa_data[4] ||
		    p_wrq->u.ap_addr.sa_data[5] )
		{
			apcli_con = 1;
		}
	}

	return apcli_con;
}

int
is_mac_in_sta_list(const unsigned char* p_mac)
{
	int i, priv_cmd;
	struct iwreq wrq;
	char mac_table_data[4096];

#if BOARD_HAS_5G_RADIO
	/* query wl for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_5G_MAIN, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq) >= 0) {
		RT_802_11_MAC_TABLE *mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
		for (i = 0; i < mp->Num; i++) {
			if (memcmp(mp->Entry[i].Addr, p_mac, ETHER_ADDR_LEN) == 0)
				return (mp->Entry[i].ApIdx == 0) ? 3 : 4;
		}
	}
#endif

#if defined(USE_RT3352_MII)
	if (nvram_get_int("inic_disable") == 1)
		return 0;
	
	if (nvram_get_int("mlme_radio_rt") == 0)
		return 0;
	
	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE;
#else
	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT;
#endif

	/* query rt for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_2G_MAIN, priv_cmd, &wrq) >= 0) {
		RT_802_11_MAC_TABLE_2G *mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;
		for (i = 0; i < mp2->Num; i++) {
			if (memcmp(mp2->Entry[i].Addr, p_mac, ETHER_ADDR_LEN) == 0)
				return (mp2->Entry[i].ApIdx == 0) ? 1 : 2;
		}
	}

	return 0;
}

int print_sta_list(webs_t wp, RT_802_11_MAC_TABLE* mp, unsigned char ApIdx)
{
	int i, ret;
	int hr, min, sec, rssi;

	ret = 0;
	if (ApIdx == 0)
		ret+=websWrite(wp, "\nAP Main Stations List\n");
	else
		ret+=websWrite(wp, "\nAP Guest Stations List\n");
	ret+=websWrite(wp, "----------------------------------------\n");
	ret+=websWrite(wp, "%-19s%-8s%-4s%-4s%-4s%-5s%-5s%-5s%-4s%-12s\n",
			   "MAC", "PhyMode", "BW", "MCS", "SGI", "STBC", "Rate", "RSSI", "PSM", "Connect Time");

	for (i=0;i<mp->Num;i++)
	{
		if (mp->Entry[i].ApIdx == ApIdx)
		{
			hr = mp->Entry[i].ConnectedTime/3600;
			min = (mp->Entry[i].ConnectedTime % 3600)/60;
			sec = mp->Entry[i].ConnectedTime - hr*3600 - min*60;
			rssi = -127;
			if ((int)mp->Entry[i].AvgRssi0 > rssi && mp->Entry[i].AvgRssi0 != 0)
				rssi = (int)mp->Entry[i].AvgRssi0;
			if ((int)mp->Entry[i].AvgRssi1 > rssi && mp->Entry[i].AvgRssi1 != 0)
				rssi = (int)mp->Entry[i].AvgRssi1;
			if ((int)mp->Entry[i].AvgRssi2 > rssi && mp->Entry[i].AvgRssi2 != 0)
				rssi = (int)mp->Entry[i].AvgRssi2;
			
			ret+=websWrite(wp, "%02X:%02X:%02X:%02X:%02X:%02X  %-7s %s %-3d %s %s  %-3dM  %-3d %s %02d:%02d:%02d\n",
				mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4], mp->Entry[i].Addr[5],
				GetPhyMode(mp->Entry[i].TxRate.field.MODE),
				GetBW(mp->Entry[i].TxRate.field.BW),
				mp->Entry[i].TxRate.field.MCS,
				mp->Entry[i].TxRate.field.ShortGI ? "Yes" : "NO ",
				mp->Entry[i].TxRate.field.STBC ? "Yes" : "NO ",
				getRate(mp->Entry[i].TxRate),
				rssi,
				mp->Entry[i].Psm ? "Yes" : "NO ",
				hr, min, sec
			);
		}
	}

	return ret;
}

int print_sta_list_2g(webs_t wp, RT_802_11_MAC_TABLE_2G* mp, unsigned char ApIdx)
{
	int i, ret;
	int hr, min, sec, rssi;

	ret = 0;
	if (ApIdx == 0)
		ret+=websWrite(wp, "\nAP Main Stations List\n");
	else
		ret+=websWrite(wp, "\nAP Guest Stations List\n");
	ret+=websWrite(wp, "----------------------------------------\n");
	ret+=websWrite(wp, "%-19s%-8s%-4s%-4s%-4s%-5s%-5s%-5s%-4s%-12s\n",
			   "MAC", "PhyMode", "BW", "MCS", "SGI", "STBC", "Rate", "RSSI", "PSM", "Connect Time");

	for (i=0;i<mp->Num;i++)
	{
		if (mp->Entry[i].ApIdx == ApIdx)
		{
			hr = mp->Entry[i].ConnectedTime/3600;
			min = (mp->Entry[i].ConnectedTime % 3600)/60;
			sec = mp->Entry[i].ConnectedTime - hr*3600 - min*60;
			rssi = -127;
			if ((int)mp->Entry[i].AvgRssi0 > rssi && mp->Entry[i].AvgRssi0 != 0)
				rssi = (int)mp->Entry[i].AvgRssi0;
			if ((int)mp->Entry[i].AvgRssi1 > rssi && mp->Entry[i].AvgRssi1 != 0)
				rssi = (int)mp->Entry[i].AvgRssi1;
			if ((int)mp->Entry[i].AvgRssi2 > rssi && mp->Entry[i].AvgRssi2 != 0)
				rssi = (int)mp->Entry[i].AvgRssi2;
			
			ret+=websWrite(wp, "%02X:%02X:%02X:%02X:%02X:%02X  %-7s %s %-3d %s %s  %-3dM  %-3d %s %02d:%02d:%02d\n",
				mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4], mp->Entry[i].Addr[5],
				GetPhyMode(mp->Entry[i].TxRate.field.MODE),
				GetBW(mp->Entry[i].TxRate.field.BW),
				mp->Entry[i].TxRate.field.MCS,
				mp->Entry[i].TxRate.field.ShortGI ? "Yes" : "NO ",
				mp->Entry[i].TxRate.field.STBC ? "Yes" : "NO ",
				getRate_2g(mp->Entry[i].TxRate),
				rssi,
				mp->Entry[i].Psm ? "Yes" : "NO ",
				hr, min, sec
			);
		}
	}

	return ret;
}

int
ej_wl_status_5g(int eid, webs_t wp, int argc, char **argv)
{
	int ret = 0;
#if BOARD_HAS_5G_RADIO
	int channel;
	int wl_mode_x;
	char *caption;
	struct iw_range	range;
	double freq;
	unsigned char mac[8];
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;

	if (nvram_match("wl_radio_x", "0")
#if defined(USE_IWPRIV_RADIO_5G)
	 || nvram_match("mlme_radio_wl", "0")
#endif
	   )
	{
		ret+=websWrite(wp, "Radio is disabled\n");
		return ret;
	}

	wl_mode_x = nvram_get_int("wl_mode_x");

	caption = (wl_mode_x == 1) ? "WDS" : "AP Main";

	if (wl_mode_x != 3)
	{
		if (wl_ioctl(IFNAME_5G_MAIN, SIOCGIWAP, &wrq0) < 0)
		{
			ret+=websWrite(wp, "Radio is disabled\n");
			return ret;
		}
		wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
		ret+=websWrite(wp, "MAC (%s)	: %02X:%02X:%02X:%02X:%02X:%02X\n", caption,
			(unsigned char)wrq0.u.ap_addr.sa_data[0],
			(unsigned char)wrq0.u.ap_addr.sa_data[1],
			(unsigned char)wrq0.u.ap_addr.sa_data[2],
			(unsigned char)wrq0.u.ap_addr.sa_data[3],
			(unsigned char)wrq0.u.ap_addr.sa_data[4],
			(unsigned char)wrq0.u.ap_addr.sa_data[5]);
	}

	if (wl_mode_x != 1 && wl_mode_x != 3 && nvram_match("wl_guest_enable", "1"))
	{
		if (wl_ioctl(IFNAME_5G_GUEST, SIOCGIWAP, &wrq0) >= 0)
		{
			wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
			ret+=websWrite(wp, "MAC (AP Guest)	: %02X:%02X:%02X:%02X:%02X:%02X\n", 
				(unsigned char)wrq0.u.ap_addr.sa_data[0],
				(unsigned char)wrq0.u.ap_addr.sa_data[1],
				(unsigned char)wrq0.u.ap_addr.sa_data[2],
				(unsigned char)wrq0.u.ap_addr.sa_data[3],
				(unsigned char)wrq0.u.ap_addr.sa_data[4],
				(unsigned char)wrq0.u.ap_addr.sa_data[5]);
		}
	}

	if (wl_mode_x == 3 || wl_mode_x == 4)
	{
		if (get_interface_hwaddr(IFNAME_5G_APCLI, mac) == 0)
		{
			ret+=websWrite(wp, "MAC (STA)	: %02X:%02X:%02X:%02X:%02X:%02X\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}

	if (wl_ioctl(IFNAME_5G_MAIN, SIOCGIWFREQ, &wrq1) < 0)
		return ret;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(IFNAME_5G_MAIN, SIOCGIWRANGE, &wrq2) < 0)
		return ret;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return ret;

	bzero(buffer, sizeof(unsigned long));
	wrq2.u.data.length = sizeof(unsigned long);
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.flags = RT_OID_GET_PHY_MODE;

	if (wl_ioctl(IFNAME_5G_MAIN, RT_PRIV_IOCTL, &wrq2) < 0)
		return ret;

#if BOARD_HAS_5G_11AC
	if (wrq2.u.mode > PHY_11VHT_N_MIXED)
#else
	if (wrq2.u.mode > PHY_11N_5G)
#endif
		phy_mode = *(unsigned long*)wrq2.u.data.pointer;
	else
		phy_mode = wrq2.u.mode;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return ret;
	}

	caption = "Operation Mode";

	if (wl_mode_x == 1)
	{
		ret+=websWrite(wp, "%s	: WDS\n", caption);
	}
	else if (wl_mode_x == 2)
	{
		if (nvram_match("wl_wdsapply_x", "1"))
			ret+=websWrite(wp, "%s	: AP & WDS\n", caption);
		else
			ret+=websWrite(wp, "%s	: AP\n", caption);
	}
	else if (wl_mode_x == 3)
	{
		ret+=websWrite(wp, "%s	: AP-Client\n", caption);
	}
	else if (wl_mode_x == 4)
	{
		ret+=websWrite(wp, "%s	: AP & AP-Client\n", caption);
	}
	else
	{
		ret+=websWrite(wp, "%s	: AP\n", caption);
	}

	caption = "HT PHY Mode";

	if (phy_mode==PHY_11A)
		ret+=websWrite(wp, "%s	: 11a\n", caption);
	else if (phy_mode==PHY_11N || phy_mode==PHY_11N_5G)
		ret+=websWrite(wp, "%s	: 11n\n", caption);
	else if (phy_mode==PHY_11AN_MIXED)
		ret+=websWrite(wp, "%s	: 11a/n\n", caption);
#if BOARD_HAS_5G_11AC
	else if (phy_mode==PHY_11VHT_N_A_MIXED)
		ret+=websWrite(wp, "%s	: 11a/n/ac\n", caption);
	else if (phy_mode==PHY_11VHT_N_MIXED)
		ret+=websWrite(wp, "%s	: 11n/ac\n", caption);
#endif

	ret+=websWrite(wp, "Channel Main	: %d\n", channel);

	if (wl_mode_x == 3 || wl_mode_x == 4)
	{
		if (get_apcli_peer_connected(IFNAME_5G_APCLI, &wrq0))
		{
			ret+=websWrite(wp, "STA Connected	: YES -> [%02X:%02X:%02X:%02X:%02X:%02X]\n",
					(unsigned char)wrq0.u.ap_addr.sa_data[0],
					(unsigned char)wrq0.u.ap_addr.sa_data[1],
					(unsigned char)wrq0.u.ap_addr.sa_data[2],
					(unsigned char)wrq0.u.ap_addr.sa_data[3],
					(unsigned char)wrq0.u.ap_addr.sa_data[4],
					(unsigned char)wrq0.u.ap_addr.sa_data[5]);
		}
		else
		{
			ret+=websWrite(wp, "STA Connected	: NO\n");
		}
	}

	if (wl_mode_x == 1 || wl_mode_x == 3)
	{
		return ret;
	}

	char mac_table_data[4096];
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq3.u.data.pointer = mac_table_data;
	wrq3.u.data.length = sizeof(mac_table_data);
	wrq3.u.data.flags = 0;

	if (wl_ioctl(IFNAME_5G_MAIN, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq3) < 0)
		return ret;

	RT_802_11_MAC_TABLE* mp=(RT_802_11_MAC_TABLE*)wrq3.u.data.pointer;
	
	ret+=print_sta_list(wp, mp, 0);
	
	if (wl_mode_x != 1 && wl_mode_x != 3 && nvram_match("wl_guest_enable", "1"))
	{
		ret+=print_sta_list(wp, mp, 1);
	}
#endif
	return ret;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char **argv)
{
	int ret = 0;
	int channel;
	int rt_mode_x;
	char *caption;
	struct iw_range	range;
	double freq;
	unsigned char mac[8];
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;

	if (nvram_match("rt_radio_x", "0")
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
	 || nvram_match("mlme_radio_rt", "0")
#endif
	   )
	{
		ret+=websWrite(wp, "Radio is disabled\n");
		return ret;
	}

	rt_mode_x = nvram_get_int("rt_mode_x");
	caption = (rt_mode_x == 1) ? "WDS" : "AP Main";

	if (rt_mode_x != 3)
	{
		if (wl_ioctl(IFNAME_2G_MAIN, SIOCGIWAP, &wrq0) < 0)
		{
			ret+=websWrite(wp, "Radio is disabled\n");
			return ret;
		}
		wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
		ret+=websWrite(wp, "MAC (%s)	: %02X:%02X:%02X:%02X:%02X:%02X\n", caption,
			(unsigned char)wrq0.u.ap_addr.sa_data[0],
			(unsigned char)wrq0.u.ap_addr.sa_data[1],
			(unsigned char)wrq0.u.ap_addr.sa_data[2],
			(unsigned char)wrq0.u.ap_addr.sa_data[3],
			(unsigned char)wrq0.u.ap_addr.sa_data[4],
			(unsigned char)wrq0.u.ap_addr.sa_data[5]);
	}

	if (rt_mode_x != 1 && rt_mode_x != 3 && nvram_match("rt_guest_enable", "1"))
	{
		if (wl_ioctl(IFNAME_2G_GUEST, SIOCGIWAP, &wrq0) >= 0)
		{
			wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
			ret+=websWrite(wp, "MAC (AP Guest)	: %02X:%02X:%02X:%02X:%02X:%02X\n", 
				(unsigned char)wrq0.u.ap_addr.sa_data[0],
				(unsigned char)wrq0.u.ap_addr.sa_data[1],
				(unsigned char)wrq0.u.ap_addr.sa_data[2],
				(unsigned char)wrq0.u.ap_addr.sa_data[3],
				(unsigned char)wrq0.u.ap_addr.sa_data[4],
				(unsigned char)wrq0.u.ap_addr.sa_data[5]);
		}
	}

	if (rt_mode_x == 3 || rt_mode_x == 4)
	{
		if (get_interface_hwaddr(IFNAME_2G_APCLI, mac) == 0)
		{
			ret+=websWrite(wp, "MAC (STA)	: %02X:%02X:%02X:%02X:%02X:%02X\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}

	if (wl_ioctl(IFNAME_2G_MAIN, SIOCGIWFREQ, &wrq1) < 0)
		return ret;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(IFNAME_2G_MAIN, SIOCGIWRANGE, &wrq2) < 0)
		return ret;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return ret;

	bzero(buffer, sizeof(unsigned long));
	wrq2.u.data.length = sizeof(unsigned long);
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.flags = RT_OID_GET_PHY_MODE;

	if (wl_ioctl(IFNAME_2G_MAIN, RT_PRIV_IOCTL, &wrq2) < 0)
		wrq2.u.mode = 0xFF;

	if (wrq2.u.mode >= PHY_11N_5G)
	{
		switch (nvram_get_int("rt_gmode"))
		{
		case 1:
			phy_mode = PHY_11BG_MIXED;
			break;
		case 5:
			phy_mode = PHY_11GN_MIXED;
			break;
		case 3:
			phy_mode = PHY_11N;
			break;
		case 4:
			phy_mode = PHY_11G;
			break;
		case 0:
			phy_mode = PHY_11B;
			break;
		default:
			phy_mode = PHY_11BGN_MIXED;
			break;
		}
	}
	else
		phy_mode = wrq2.u.mode;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else
	{
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return ret;
	}

	caption = "Operation Mode";
	if (rt_mode_x == 1)
	{
		ret+=websWrite(wp, "%s	: WDS\n", caption);
	}
	else if (rt_mode_x == 2)
	{
		if (nvram_match("rt_wdsapply_x", "1"))
			ret+=websWrite(wp, "%s	: AP & WDS\n", caption);
		else
			ret+=websWrite(wp, "%s	: AP\n", caption);
	}
	else if (rt_mode_x == 3)
	{
		ret+=websWrite(wp, "%s	: AP-Client\n", caption);
	}
	else if (rt_mode_x == 4)
	{
		ret+=websWrite(wp, "%s	: AP & AP-Client\n", caption);
	}
	else
	{
		ret+=websWrite(wp, "%s	: AP\n", caption);
	}

	caption = "HT PHY Mode";
	if (phy_mode==PHY_11BG_MIXED)
		ret+=websWrite(wp, "%s	: 11b/g\n", caption);
	else if (phy_mode==PHY_11B)
		ret+=websWrite(wp, "%s	: 11b\n", caption);
	else if (phy_mode==PHY_11A)
		ret+=websWrite(wp, "%s	: 11a\n", caption);
	else if (phy_mode==PHY_11ABG_MIXED)
		ret+=websWrite(wp, "%s	: 11a/b/g\n", caption);
	else if (phy_mode==PHY_11G)
		ret+=websWrite(wp, "%s	: 11g\n", caption);
	else if (phy_mode==PHY_11ABGN_MIXED)
		ret+=websWrite(wp, "%s	: 11a/b/g/n\n", caption);
	else if (phy_mode==PHY_11N)
		ret+=websWrite(wp, "%s	: 11n\n", caption);
	else if (phy_mode==PHY_11GN_MIXED)
		ret+=websWrite(wp, "%s	: 11g/n\n", caption);
	else if (phy_mode==PHY_11AN_MIXED)
		ret+=websWrite(wp, "%s	: 11a/n\n", caption);
	else if (phy_mode==PHY_11BGN_MIXED)
		ret+=websWrite(wp, "%s	: 11b/g/n\n", caption);
	else if (phy_mode==PHY_11AGN_MIXED)
		ret+=websWrite(wp, "%s	: 11a/g/n\n", caption);

	ret+=websWrite(wp, "Channel Main	: %d\n", channel);

	if (rt_mode_x == 3 || rt_mode_x == 4)
	{
		if (get_apcli_peer_connected(IFNAME_2G_APCLI, &wrq0))
		{
			ret+=websWrite(wp, "STA Connected	: YES -> [%02X:%02X:%02X:%02X:%02X:%02X]\n",
					(unsigned char)wrq0.u.ap_addr.sa_data[0],
					(unsigned char)wrq0.u.ap_addr.sa_data[1],
					(unsigned char)wrq0.u.ap_addr.sa_data[2],
					(unsigned char)wrq0.u.ap_addr.sa_data[3],
					(unsigned char)wrq0.u.ap_addr.sa_data[4],
					(unsigned char)wrq0.u.ap_addr.sa_data[5]);
		}
		else
		{
			ret+=websWrite(wp, "STA Connected	: NO\n");
		}
	}

	if (rt_mode_x == 1 || rt_mode_x == 3)
	{
		return ret;
	}

	int priv_cmd;
	char mac_table_data[4096];
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq3.u.data.pointer = mac_table_data;
	wrq3.u.data.length = sizeof(mac_table_data);
	wrq3.u.data.flags = 0;

#if defined(USE_RT3352_MII)
	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE;
#else
	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT;
#endif

	if (wl_ioctl(IFNAME_2G_MAIN, priv_cmd, &wrq3) < 0)
		return ret;

	RT_802_11_MAC_TABLE_2G* mp=(RT_802_11_MAC_TABLE_2G*)wrq3.u.data.pointer;
	
	ret+=print_sta_list_2g(wp, mp, 0);
	
	if (rt_mode_x != 1 && rt_mode_x != 3 && nvram_match("rt_guest_enable", "1"))
	{
		ret+=print_sta_list_2g(wp, mp, 1);
	}

	return ret;
}

int 
ej_wl_auth_list(int eid, webs_t wp, int argc, char **argv)
{
	struct iwreq wrq;
	int i, priv_cmd, firstRow = 1, ret = 0;
	char mac_table_data[4096];
	char mac[18];

#if BOARD_HAS_5G_RADIO
	/* query wl for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_5G_MAIN, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq) >= 0)
	{
		RT_802_11_MAC_TABLE *mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
		for (i=0; i<mp->Num; i++)
		{
			sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4], mp->Entry[i].Addr[5]);
			
			if (firstRow)
				firstRow = 0;
			else
				ret+=websWrite(wp, ", ");
			
			ret+=websWrite(wp, "\"%s\"", mac);
		}
	}
#endif

#if defined(USE_RT3352_MII)
	if (nvram_get_int("inic_disable") == 1)
		return ret;

	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE;
#else
	priv_cmd = RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT;
#endif

	/* query rt for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_2G_MAIN, priv_cmd, &wrq) >= 0)
	{
		RT_802_11_MAC_TABLE_2G *mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;
		for (i = 0; i<mp2->Num; i++)
		{
			sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				mp2->Entry[i].Addr[0], mp2->Entry[i].Addr[1],
				mp2->Entry[i].Addr[2], mp2->Entry[i].Addr[3],
				mp2->Entry[i].Addr[4], mp2->Entry[i].Addr[5]);
			
			if (firstRow)
				firstRow = 0;
			else
				ret+=websWrite(wp, ", ");
			
			ret+=websWrite(wp, "\"%s\"", mac);
		}
	}

	return ret;
}


#define SSURV_LINE_LEN		(4+33+20+23+9+7+7+3)		// Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType
#define SSURV_LINE_LEN_WPS	(4+33+20+23+9+7+7+3+4+5)	// Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType+WPS+PIN

int 
ej_wl_scan_5g(int eid, webs_t wp, int argc, char **argv)
{
#if BOARD_HAS_5G_RADIO
	int retval = 0;
	int apCount = 0;
	char data[8192];
	char ssid_str[128];
#if defined(USE_WSC_WPS)
	char site_line[SSURV_LINE_LEN_WPS+1];
#else
	char site_line[SSURV_LINE_LEN+1];
#endif
	char site_chnl[4];
	char site_ssid[34];
	char site_bssid[24];
	char site_signal[10];
	struct iwreq wrq;
	char *sp, *op, *empty;
	int len, line_len;

	empty = "[\"\", \"\", \"\", \"\"]";

	memset(data, 0, 32);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(IFNAME_5G_MAIN, RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		dbg("Site Survey fails\n");
		return websWrite(wp, "[%s]", empty);
	}

	dbg("Please wait...\n\n");
	sleep(4);

	memset(data, 0, sizeof(data));
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_5G_MAIN, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return websWrite(wp, "[%s]", empty);
	}

#if defined(USE_WSC_WPS)
	line_len = SSURV_LINE_LEN_WPS;
//	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s%-4s%-5s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH", "NT", "WPS", "DPID");
#else
	line_len = SSURV_LINE_LEN;
//	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH", "NT");
#endif

	retval += websWrite(wp, "[");
	if (wrq.u.data.length > 0)
	{
		op = sp = wrq.u.data.pointer+line_len+2; // skip \n+\n
		len = strlen(op);
		
		while (*sp && ((len - (sp-op)) >= 0))
		{
			memcpy(site_line, sp, line_len);
			
			memcpy(site_chnl, sp, 3);
			memcpy(site_ssid, sp+4, 33);
			memcpy(site_bssid, sp+37, 20);
			memcpy(site_signal, sp+80, 9);
			
			site_line[line_len] = '\0';
			site_chnl[3] = '\0';
			site_ssid[33] = '\0';
			site_bssid[20] = '\0';
			site_signal[9] = '\0';
			
			memset(ssid_str, 0, sizeof(ssid_str));
			char_to_ascii(ssid_str, trim_r(site_ssid));
			
			if (!strlen(ssid_str))
				strcpy(ssid_str, "???");
			
			if (apCount)
				retval += websWrite(wp, "%s ", ",");
			
			retval += websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_chnl), trim_r(site_signal));
			
//			dbg("%s\n", site_line);
			
			sp+=line_len+1; // skip \n
			apCount++;
		}
	}

	if (apCount < 1)
	{
		retval += websWrite(wp, empty);
	}

	retval += websWrite(wp, "]");

	return retval;
#else
	return websWrite(wp, "[%s]", "[\"\", \"\", \"\", \"\"]");
#endif
}

int 
ej_wl_scan_2g(int eid, webs_t wp, int argc, char **argv)
{
	int retval = 0, apCount = 0;
	char data[8192];
	char ssid_str[128];
#if defined(USE_WSC_WPS) || defined(USE_RT3352_MII)
	char site_line[SSURV_LINE_LEN_WPS+1];
#else
	char site_line[SSURV_LINE_LEN+1];
#endif
	char site_chnl[4];
	char site_ssid[34];
	char site_bssid[24];
	char site_signal[10];
	struct iwreq wrq;
	char *sp, *op, *empty;
	int len, line_len;

	empty = "[\"\", \"\", \"\", \"\"]";

	memset(data, 0, 32);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(IFNAME_2G_MAIN, RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		dbg("Site Survey fails\n");
		return websWrite(wp, "[%s]", empty);
	}

	dbg("Please wait...\n\n");
	sleep(4);

	memset(data, 0, sizeof(data));
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	if (wl_ioctl(IFNAME_2G_MAIN, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return websWrite(wp, "[%s]",empty);
	}

#if defined(USE_WSC_WPS) || defined(USE_RT3352_MII)
	line_len = SSURV_LINE_LEN_WPS;
//	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s%-4s%-5s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH", "NT", "WPS", "DPID");
#else
	line_len = SSURV_LINE_LEN;
//	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH", "NT");
#endif
	retval += websWrite(wp, "[");
	if (wrq.u.data.length > 0)
	{
		op = sp = wrq.u.data.pointer+line_len+2; // skip \n+\n
		len = strlen(op);
		
		while (*sp && ((len - (sp-op)) >= 0))
		{
			memcpy(site_line, sp, line_len);
			
			memcpy(site_chnl, sp, 3);
			memcpy(site_ssid, sp+4, 33);
			memcpy(site_bssid, sp+37, 20);
			memcpy(site_signal, sp+80, 9);
			
			site_line[line_len] = '\0';
			site_chnl[3] = '\0';
			site_ssid[33] = '\0';
			site_bssid[20] = '\0';
			site_signal[9] = '\0';
			
			memset(ssid_str, 0, sizeof(ssid_str));
			char_to_ascii(ssid_str, trim_r(site_ssid));
			
			if (!strlen(ssid_str))
				strcpy(ssid_str, "???");
			
			if (apCount)
				retval += websWrite(wp, "%s ", ",");
			
			retval += websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_chnl), trim_r(site_signal));
			
//			dbg("%s\n", site_line);
			
			sp+=line_len+1; // skip \n
			apCount++;
		}
	}

	if (apCount < 1)
	{
		retval += websWrite(wp, empty);
	}

	retval += websWrite(wp, "]");

	return retval;
}

int 
ej_wl_bssid_5g(int eid, webs_t wp, int argc, char **argv)
{
	char bssid[32] = {0};
#if BOARD_HAS_5G_RADIO
	unsigned char mac[8];
#endif

	snprintf(bssid, sizeof(bssid), "%s", nvram_safe_get("wl_macaddr"));
#if BOARD_HAS_5G_RADIO
	if (get_interface_hwaddr(IFNAME_5G_MAIN, mac) == 0)
	{
		sprintf(bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
#endif

	websWrite(wp, "function get_bssid_ra0() { return '%s';}\n", bssid);

	return 0;
}

int 
ej_wl_bssid_2g(int eid, webs_t wp, int argc, char **argv)
{
	char bssid[32] = {0};
	unsigned char mac[8];

	snprintf(bssid, sizeof(bssid), "%s", nvram_safe_get("rt_macaddr"));

#if defined(USE_RT3352_MII)
	if (nvram_get_int("inic_disable") != 1)
	{
#endif
		if (get_interface_hwaddr(IFNAME_2G_MAIN, mac) == 0)
		{
			sprintf(bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
#if defined(USE_RT3352_MII)
	}
#endif

	websWrite(wp, "function get_bssid_rai0() { return '%s';}\n", bssid);

	return 0;
}

