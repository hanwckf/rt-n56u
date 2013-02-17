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
/*
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "httpd.h"

#include <net/ethernet.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <netconf.h>
#include <ralink.h>
#include <iwlib.h>
#include "stapriv.h"

void reltime(unsigned int seconds, char *buf);

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#include <dirent.h>

#include "common.h"

/******************************************************************************************************************************************/

/* Dump leases in <tr><td>hostname</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
int
ej_lan_leases(int eid, webs_t wp, int argc, char_t **argv)
{
	/* dnsmasq ex: 43200 00:26:18:57:08:bc 192.168.1.105 mypc-3eaf6880a0 01:00:26:18:57:08:bc */
	
	FILE *fp = NULL;
	int ret = 0;
	int i;
	char buff[256], dh_lease[32], dh_mac[32], dh_ip[32], dh_host[64], dh_sid[32];
	
	ret += websWrite(wp, "MAC Address         IP Address        Host Name\n");
	
	if (!(fp = fopen("/tmp/dnsmasq.leases", "r")))
		return ret;
	
	buff[0] = 0;
	while (fgets(buff, sizeof(buff), fp))
	{
		dh_lease[0] = 0;
		dh_mac[0] = 0;
		dh_ip[0] = 0;
		dh_host[0] = 0;
		dh_sid[0] = 0;
		if (sscanf(buff, "%s %s %s %s %s", dh_lease, dh_mac, dh_ip, dh_host, dh_sid) != 5)
			continue;
		
		strcat(dh_lease, " secs");
		
		if (!dh_host[0])
			strcpy(dh_host, "*");
		
		// convert MAC to upper case
		for (i=0; i<strlen(dh_mac); i++)
			dh_mac[i] = toupper(dh_mac[i]);
		
		ret += websWrite(wp, "%-20s", (dh_mac[0]!=0) ? dh_mac : " " );
		ret += websWrite(wp, "%-18s", (dh_ip[0]!=0) ? dh_ip : " ");
		ret += websWrite(wp, "%s\n",  dh_host);
		
		buff[0] = 0;
	}
	fclose(fp);
	
	return ret;
}

int
ej_vpns_leases(int eid, webs_t wp, int argc, char_t **argv)
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

#if !defined (USE_KERNEL3X)
int is_swnat_loaded()
{
	FILE *fp;
	char nf_value[32];
	
	fp = fopen("/proc/sys/net/nf_conntrack_fastnat", "r");
	if (fp) {
		nf_value[0] = 0;
		fgets(nf_value, sizeof(nf_value), fp);
		fclose(fp);
		if (strlen(nf_value) > 0) {
			nf_value[strlen(nf_value) - 1] = 0;
			return atoi(nf_value);
		}
	}
	
	return 0;
}
#endif

/* Dump NAT table <tr><td>destination</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
int
ej_nat_table(int eid, webs_t wp, int argc, char_t **argv)
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
			hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/Wi-Fi]";
		else if (i_loaded == 1)
			hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN]";
		
		ret += websWrite(wp, "Hardware NAT/Routing: %s\n", hwnat_status);
	}
	
	if (sw_mode == 1)
	{
#if !defined (USE_KERNEL3X)
		char *swnat_status = "Disabled";
		if (is_swnat_loaded())
			swnat_status = "Enabled";
		ret += websWrite(wp, "Software FastNAT: %s\n", swnat_status);
#endif
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
ej_route_table(int eid, webs_t wp, int argc, char_t **argv)
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

#define RTPRIV_IOCTL_SHOW		SIOCIWFIRSTPRIV + 0x11
#define RTPRIV_IOCTL_GET_MAC_TABLE	SIOCIWFIRSTPRIV + 0x0F

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
		default:
			return "N/A";
	}
}

int MCSMappingRateTable[] =
	{2,  4,   11,  22, // CCK
	12, 18,   24,  36, 48, 72, 96, 108, // OFDM
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, // 20MHz, 800ns GI, MCS: 0 ~ 15
	39, 78,  117, 156, 234, 312, 351, 390,										  // 20MHz, 800ns GI, MCS: 16 ~ 23
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, // 40MHz, 800ns GI, MCS: 0 ~ 15
	81, 162, 243, 324, 486, 648, 729, 810,										  // 40MHz, 800ns GI, MCS: 16 ~ 23
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, // 20MHz, 400ns GI, MCS: 0 ~ 15
	43, 87,  130, 173, 260, 317, 390, 433,										  // 20MHz, 400ns GI, MCS: 16 ~ 23
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, // 40MHz, 400ns GI, MCS: 0 ~ 15
	90, 180, 270, 360, 540, 720, 810, 900};

int
getRate(MACHTTRANSMIT_SETTING HTSetting)
{
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;  

    if (HTSetting.field.MODE >= MODE_HTMIX)
    {
    	rate_index = 12 + ((unsigned char)HTSetting.field.BW *24) + ((unsigned char)HTSetting.field.ShortGI *48) + ((unsigned char)HTSetting.field.MCS);
    }
    else 
    if (HTSetting.field.MODE == MODE_OFDM)                
    	rate_index = (unsigned char)(HTSetting.field.MCS) + 4;
    else if (HTSetting.field.MODE == MODE_CCK)   
    	rate_index = (unsigned char)(HTSetting.field.MCS);

    if (rate_index < 0)
        rate_index = 0;
    
    if (rate_index > rate_count)
        rate_index = rate_count;

	return (MCSMappingRateTable[rate_index] * 5)/10;
}

int
getRate_2g(MACHTTRANSMIT_SETTING_2G HTSetting)
{
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;  

    if (HTSetting.field.MODE >= MODE_HTMIX)
    {
    	rate_index = 12 + ((unsigned char)HTSetting.field.BW *24) + ((unsigned char)HTSetting.field.ShortGI *48) + ((unsigned char)HTSetting.field.MCS);
    }
    else 
    if (HTSetting.field.MODE == MODE_OFDM)                
    	rate_index = (unsigned char)(HTSetting.field.MCS) + 4;
    else if (HTSetting.field.MODE == MODE_CCK)   
    	rate_index = (unsigned char)(HTSetting.field.MCS);

    if (rate_index < 0)
        rate_index = 0;
    
    if (rate_index > rate_count)
        rate_index = rate_count;

	return (MCSMappingRateTable[rate_index] * 5)/10;
}

int get_if_hwaddr(char *ifname, struct ifreq *p_ifr)
{
	int sockfd;
	int result = 0;
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		return -1;
	}
	
	memset(p_ifr, 0, sizeof(struct ifreq));
	strncpy(p_ifr->ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, p_ifr) < 0) {
		result = -1;
	}
	
	close(sockfd);
	
	return result;
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
ej_wl_status_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	int channel;
	int wl_mode_x;
	int connected;
	char *caption;
	struct iw_range	range;
	double freq;
	struct ifreq ifr;
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;

	if (nvram_match("wl_radio_x", "0"))
	{
		ret+=websWrite(wp, "Radio is disabled\n");
		return ret;
	}

	wl_mode_x = nvram_get_int("wl_mode_x");

	caption = (wl_mode_x == 1) ? "WDS" : "AP Main";

	if (wl_mode_x != 3)
	{
		if (wl_ioctl(WIF, SIOCGIWAP, &wrq0) < 0)
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
		if (wl_ioctl("ra1", SIOCGIWAP, &wrq0) >= 0)
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
		if (get_if_hwaddr("apcli0", &ifr) == 0)
		{
			ret+=websWrite(wp, "MAC (STA)	: %02X:%02X:%02X:%02X:%02X:%02X\n",
				(unsigned char)ifr.ifr_hwaddr.sa_data[0],
				(unsigned char)ifr.ifr_hwaddr.sa_data[1],
				(unsigned char)ifr.ifr_hwaddr.sa_data[2],
				(unsigned char)ifr.ifr_hwaddr.sa_data[3],
				(unsigned char)ifr.ifr_hwaddr.sa_data[4],
				(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		}
	}

	if (wl_ioctl(WIF, SIOCGIWFREQ, &wrq1) < 0)
		return ret;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF, SIOCGIWRANGE, &wrq2) < 0)
		return ret;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return ret;

	bzero(buffer, sizeof(unsigned long));
	wrq2.u.data.length = sizeof(unsigned long);
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.flags = RT_OID_GET_PHY_MODE;

	if (wl_ioctl(WIF, RT_PRIV_IOCTL, &wrq2) < 0)
		return ret;

	if (wrq2.u.mode > PHY_11N_5G)
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
	else if (phy_mode==PHY_11N || phy_mode==PHY_11N_5G)
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

	if (wl_mode_x == 3 || wl_mode_x == 4)
	{
		connected = 0;
		if (wl_ioctl("apcli0", SIOCGIWAP, &wrq0) >= 0)
		{
			wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
			if (wrq0.u.ap_addr.sa_data[0] ||
			    wrq0.u.ap_addr.sa_data[1] ||
			    wrq0.u.ap_addr.sa_data[2] ||
			    wrq0.u.ap_addr.sa_data[3] ||
			    wrq0.u.ap_addr.sa_data[4] ||
			    wrq0.u.ap_addr.sa_data[5] )
			{
				connected = 1;
			}
		}
		
		if (connected)
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

	if (wl_ioctl(WIF, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq3) < 0)
		return ret;

	RT_802_11_MAC_TABLE* mp=(RT_802_11_MAC_TABLE*)wrq3.u.data.pointer;
	
	ret+=print_sta_list(wp, mp, 0);
	
	if (wl_mode_x != 1 && wl_mode_x != 3 && nvram_match("wl_guest_enable", "1"))
	{
		ret+=print_sta_list(wp, mp, 1);
	}

	return ret;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	int channel;
	int rt_mode_x;
	int connected;
	char *caption;
	struct iw_range	range;
	double freq;
	struct ifreq ifr;
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;

	if (nvram_match("rt_radio_x", "0"))
	{
		ret+=websWrite(wp, "Radio is disabled\n");
		return ret;
	}

	rt_mode_x = nvram_get_int("rt_mode_x");
	caption = (rt_mode_x == 1) ? "WDS" : "AP Main";

	if (rt_mode_x != 3)
	{
		if (wl_ioctl(WIF2G, SIOCGIWAP, &wrq0) < 0)
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
		if (wl_ioctl("rai1", SIOCGIWAP, &wrq0) >= 0)
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
		if (get_if_hwaddr(IFNAME_INIC_APCLI, &ifr) == 0)
		{
			ret+=websWrite(wp, "MAC (STA)	: %02X:%02X:%02X:%02X:%02X:%02X\n",
				(unsigned char)ifr.ifr_hwaddr.sa_data[0],
				(unsigned char)ifr.ifr_hwaddr.sa_data[1],
				(unsigned char)ifr.ifr_hwaddr.sa_data[2],
				(unsigned char)ifr.ifr_hwaddr.sa_data[3],
				(unsigned char)ifr.ifr_hwaddr.sa_data[4],
				(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		}
	}

	if (wl_ioctl(WIF2G, SIOCGIWFREQ, &wrq1) < 0)
		return ret;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(WIF2G, SIOCGIWRANGE, &wrq2) < 0)
		return ret;

	if (ralink_get_range_info(&range, buffer, wrq2.u.data.length) < 0)
		return ret;

	bzero(buffer, sizeof(unsigned long));
	wrq2.u.data.length = sizeof(unsigned long);
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.flags = RT_OID_GET_PHY_MODE;

	if (wl_ioctl(WIF2G, RT_PRIV_IOCTL, &wrq2) < 0)
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
		connected = 0;
		if (wl_ioctl(IFNAME_INIC_APCLI, SIOCGIWAP, &wrq0) >= 0)
		{
			wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
			if (wrq0.u.ap_addr.sa_data[0] ||
			    wrq0.u.ap_addr.sa_data[1] ||
			    wrq0.u.ap_addr.sa_data[2] ||
			    wrq0.u.ap_addr.sa_data[3] ||
			    wrq0.u.ap_addr.sa_data[4] ||
			    wrq0.u.ap_addr.sa_data[5] )
			{
				connected = 1;
			}
		}
		
		if (connected)
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

	char mac_table_data[4096];
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq3.u.data.pointer = mac_table_data;
	wrq3.u.data.length = sizeof(mac_table_data);
	wrq3.u.data.flags = 0;

	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq3) < 0)
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
ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
	struct iwreq wrq;
	int i, firstRow, ret = 0;
	char mac_table_data[4096];
	char mac[18];
	RT_802_11_MAC_TABLE *mp;
	RT_802_11_MAC_TABLE_2G *mp2;
	
	firstRow = 1;
	
	/* query wl for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) >= 0)
	{
		mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
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
			
			ret+=websWrite(wp, "[");
			ret+=websWrite(wp, "\"%s\"", mac);
			ret+=websWrite(wp, ", \"%s\"", "YES");
			ret+=websWrite(wp, ", \"%s\"", "");
			ret+=websWrite(wp, "]");
		}
	}

	/* query rt for authenticated sta list */
	memset(mac_table_data, 0, sizeof(mac_table_data));
	wrq.u.data.pointer = mac_table_data;
	wrq.u.data.length = sizeof(mac_table_data);
	wrq.u.data.flags = 0;
	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) >= 0)
	{
		mp2 = (RT_802_11_MAC_TABLE_2G *)wrq.u.data.pointer;
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
			
			ret+=websWrite(wp, "[");
			ret+=websWrite(wp, "\"%s\"", mac);
			ret+=websWrite(wp, ", \"%s\"", "YES");
			ret+=websWrite(wp, ", \"%s\"", "");
			ret+=websWrite(wp, "]");
		}
	}

	return ret;
}


#define SSURV_LINE_LEN		(4+33+20+23+9+7+7+3)		// Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType
#define SSURV_LINE_LEN_WPS	(4+33+20+23+9+7+7+3+4+5)	// Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType+WPS+PIN

int 
ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0, apCount = 0;
	char data[8192];
	char ssid_str[128];
	char site_line[SSURV_LINE_LEN+1];
	char site_ssid[34];
	char site_bssid[24];
	char site_signal[10];
	struct iwreq wrq;
	char *sp, *op, *empty;
	int len;

	empty = "[\"\", \"\", \"\"]";

	memset(data, 0, 32);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(WIF, RTPRIV_IOCTL_SET, &wrq) < 0)
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
	if (wl_ioctl(WIF, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return websWrite(wp, "[%s]", empty);
	}
	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH"," NT");

	retval += websWrite(wp, "[");
	if (wrq.u.data.length > 0)
	{
		op = sp = wrq.u.data.pointer+SSURV_LINE_LEN+2; // skip \n+\n
		len = strlen(op);
		
		while (*sp && ((len - (sp-op)) >= 0))
		{
			memcpy(site_line, sp, SSURV_LINE_LEN);
			memcpy(site_ssid, sp+4, 33);
			memcpy(site_bssid, sp+37, 20);
			memcpy(site_signal, sp+80, 9);
			
			site_line[SSURV_LINE_LEN] = '\0';
			site_ssid[33] = '\0';
			site_bssid[20] = '\0';
			site_signal[9] = '\0';
			
			memset(ssid_str, 0, sizeof(ssid_str));
			char_to_ascii(ssid_str, trim_r(site_ssid));
			
			if (!strlen(ssid_str))
				strcpy(ssid_str, "???");
			
			if (!apCount)
				retval += websWrite(wp, "[\"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_signal));
			else
				retval += websWrite(wp, ", [\"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_signal));
			
			dbg("%s\n", site_line);
			
			sp+=SSURV_LINE_LEN+1; // skip \n
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
ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0, apCount = 0;
	char data[8192];
	char ssid_str[128];
#if defined(USE_RT3352_MII)
	char site_line[SSURV_LINE_LEN_WPS+1];
#else
	char site_line[SSURV_LINE_LEN+1];
#endif
	char site_ssid[34];
	char site_bssid[24];
	char site_signal[10];
	struct iwreq wrq;
	char *sp, *op, *empty;
	int len, line_len;

	empty = "[\"\", \"\", \"\"]";

	memset(data, 0, 32);
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_SET, &wrq) < 0)
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
	if (wl_ioctl(WIF2G, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return websWrite(wp, "[%s]",empty);
	}

#if defined(USE_RT3352_MII)
	line_len = SSURV_LINE_LEN_WPS;
	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s%-4s%-5s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", "ExtCH", "NT", "WPS", "DPID");
#else
	line_len = SSURV_LINE_LEN;
	dbg("%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s\n", "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", "ExtCH", "NT");
#endif
	retval += websWrite(wp, "[");
	if (wrq.u.data.length > 0)
	{
		op = sp = wrq.u.data.pointer+line_len+2; // skip \n+\n
		len = strlen(op);
		
		while (*sp && ((len - (sp-op)) >= 0))
		{
			memcpy(site_line, sp, line_len);
			memcpy(site_ssid, sp+4, 33);
			memcpy(site_bssid, sp+37, 20);
			memcpy(site_signal, sp+80, 9);
			
			site_line[line_len] = '\0';
			site_ssid[33] = '\0';
			site_bssid[20] = '\0';
			site_signal[9] = '\0';
			
			memset(ssid_str, 0, sizeof(ssid_str));
			char_to_ascii(ssid_str, trim_r(site_ssid));
			
			if (!strlen(ssid_str))
				strcpy(ssid_str, "???");
			
			if (!apCount)
				retval += websWrite(wp, "[\"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_signal));
			else
				retval += websWrite(wp, ", [\"%s\", \"%s\", \"%s\"]", ssid_str, trim_r(site_bssid), trim_r(site_signal));
			
			dbg("%s\n", site_line);
			
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
ej_wl_bssid_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	struct ifreq ifr;
	char bssid[32] = {0};
	const char *fmt_mac = "%02X:%02X:%02X:%02X:%02X:%02X";
	
	strcpy(bssid, nvram_safe_get("il0macaddr"));
	if (get_if_hwaddr(WIF, &ifr) == 0)
	{
		sprintf(bssid, fmt_mac,
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
	}
	
	websWrite(wp, "function get_bssid_ra0() { return '%s';}\n", bssid);
	
	dbg("ej_wl_bssid_5g!\n");
	
	return 0;
}

int 
ej_wl_bssid_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	struct ifreq ifr;
	char bssid[32] = {0};
	const char *fmt_mac = "%02X:%02X:%02X:%02X:%02X:%02X";
	
	strcpy(bssid, nvram_safe_get("il1macaddr"));
	if (get_if_hwaddr(WIF2G, &ifr) == 0)
	{
		sprintf(bssid, fmt_mac,
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
	}
	
	websWrite(wp, "function get_bssid_rai0() { return '%s';}\n", bssid);
	
	dbg("ej_wl_bssid_2g!\n");
	
	return 0;
}

