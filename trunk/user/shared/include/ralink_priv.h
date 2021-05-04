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

#ifndef _ralink_h_
#define _ralink_h_

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

#if defined (CONFIG_RT_MAX_CLIENTS)
#define MAX_NUMBER_OF_MAC	CONFIG_RT_MAX_CLIENTS
#else
#define MAX_NUMBER_OF_MAC	32
#endif

#define FIXED_TXMODE_HT		0
#define FIXED_TXMODE_CCK	1
#define FIXED_TXMODE_OFDM 	2
#define FIXED_TXMODE_VHT	3

#define MODE_CCK		0
#define MODE_OFDM		1
#define MODE_HTMIX		2
#define MODE_HTGREENFIELD	3
#define MODE_VHT		4
#define MODE_HE		5
#define MODE_HE_SU		8
#define MODE_HE_24G		7
#define MODE_HE_5G		6
#define MODE_HE_EXT_SU		9
#define MODE_HE_TRIG		10
#define MODE_HE_MU		11
#define MODE_UNKNOWN		255

#define BW_20			0
#define BW_40			1
#define BW_80			2
#define BW_160			3
#define BW_10			4
#define BW_5			5
#define BW_8080		6

#define WDS_DISABLE_MODE	0
#define WDS_RESTRICT_MODE	1
#define WDS_BRIDGE_MODE		2
#define WDS_REPEATER_MODE	3
#define WDS_LAZY_MODE		4

typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B,
	PHY_11A,
	PHY_11ABG_MIXED,
	PHY_11G,
	PHY_11ABGN_MIXED,   // both band   5
	PHY_11N,            //    6
	PHY_11GN_MIXED,     // 2.4G band      7
	PHY_11AN_MIXED,     // 5G  band       8
	PHY_11BGN_MIXED,    // if check 802.11b.      9
	PHY_11AGN_MIXED,    // if check 802.11b.      10
	PHY_11N_5G,         // 11n-only with 5G band  11
	PHY_11VHT_N_ABG_MIXED = 12, /* 12 -> AC/A/AN/B/G/GN mixed */
	PHY_11VHT_N_AG_MIXED = 13, /* 13 -> AC/A/AN/G/GN mixed  */
	PHY_11VHT_N_A_MIXED = 14, /* 14 -> AC/AN/A mixed in 5G band */
	PHY_11VHT_N_MIXED = 15, /* 15 -> AC/AN mixed in 5G band */
	PHY_11AX_24G = 16,
	PHY_11AX_5G = 17,
	PHY_11AX_6G = 18,
	PHY_11AX_24G_6G = 19,
	PHY_11AX_5G_6G = 20,
	PHY_11AX_24G_5G_6G = 21,
} RT_802_11_PHY_MODE;

enum WIFI_MODE {
	WMODE_INVALID = 0,
	WMODE_A = 1 << 0,
	WMODE_B = 1 << 1,
	WMODE_G = 1 << 2,
	WMODE_GN = 1 << 3,
	WMODE_AN = 1 << 4,
	WMODE_AC = 1 << 5,
	WMODE_AX_24G = 1 << 6,
	WMODE_AX_5G = 1 << 7,
	WMODE_AX_6G = 1 << 8,
	WMODE_COMP = 9,	/* total types of supported wireless mode, add this value once yow add new type */
};

typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS:6;
		unsigned short ldpc:1;
		unsigned short BW:2;
		unsigned short ShortGI:1;
		unsigned short STBC:1;
		unsigned short eTxBF:1;
		unsigned short iTxBF:1;
		unsigned short MODE:3;
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char	ApIdx;
	unsigned char	Addr[ETHER_ADDR_LEN];
	unsigned char	Aid;
	unsigned char	Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char	MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char		AvgRssi0;
	char		AvgRssi1;
	char		AvgRssi2;
	unsigned int	ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int	LastRxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

/* RT3352 iNIC_mii MAC_TABLE */
typedef union _MACHTTRANSMIT_SETTING_INIC {
	struct {
		unsigned short MCS:7;
		unsigned short BW:1;
		unsigned short ShortGI:1;
		unsigned short STBC:2;
		unsigned short eTxBF:1;
		unsigned short rsv:1;
		unsigned short iTxBF:1;
		unsigned short MODE:2;
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING_INIC, *PMACHTTRANSMIT_SETTING_INIC;

typedef struct _RT_802_11_MAC_ENTRY_INIC {
	unsigned char	ApIdx;
	unsigned char	Addr[ETHER_ADDR_LEN];
	unsigned char	Aid;
	unsigned char	Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char	MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char		AvgRssi0;
	char		AvgRssi1;
	char		AvgRssi2;
	unsigned int	ConnectedTime;
	MACHTTRANSMIT_SETTING_INIC	TxRate;
} RT_802_11_MAC_ENTRY_INIC, *PRT_802_11_MAC_ENTRY_INIC;

typedef struct _RT_802_11_MAC_TABLE_INIC {
	unsigned long Num;
	RT_802_11_MAC_ENTRY_INIC Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE_INIC, *PRT_802_11_MAC_TABLE_INIC;

typedef struct _SITE_SURVEY 
{
	char channel[4];
	unsigned char ssid[33];
	char bssid[20];
	char security[23];
	char signal[9];
	char wmode[7];
	char extch[7];
	char bsstype[3];
} SITE_SURVEY;

typedef struct _SITE_SURVEY_ARRAY
{
	SITE_SURVEY SiteSurvey[64];
} SSA;

#define SITE_SURVEY_APS_MAX	(16*1024)

typedef struct _PAIR_CHANNEL_FREQ_ENTRY
{
	unsigned long   lChannel;
	unsigned long   lFreq;
} PAIR_CHANNEL_FREQ_ENTRY, *PPAIR_CHANNEL_FREQ_ENTRY;


//#if WIRELESS_EXT <= 11 
//#ifndef SIOCDEVPRIVATE 
//#define SIOCDEVPRIVATE 0x8BE0 
//#endif 
//#define SIOCIWFIRSTPRIV SIOCDEVPRIVATE 
//#endif 
//
//SET/GET CONVENTION :
// * ------------------
// * Simplistic summary :
// * o even numbered ioctls are SET, restricted to root, and should not
// * return arguments (get_args = 0).
// * o odd numbered ioctls are GET, authorised to anybody, and should
// * not expect any arguments (set_args = 0).
//
#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET		(SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_GSITESURVEY	(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_WSC_PROFILE	(SIOCIWFIRSTPRIV + 0x12)
#define	RTPRIV_IOCTL_GSTAINFO		(SIOCIWFIRSTPRIV + 0x1A)
#define	RTPRIV_IOCTL_GSTAT		(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GRSSI		(SIOCIWFIRSTPRIV + 0x1C)
#define RTPRIV_IOCTL_GTXBFCALP		(SIOCIWFIRSTPRIV + 0x1D)
#define OID_802_11_DISASSOCIATE		0x0114
#define OID_802_11_BSSID_LIST_SCAN	0x0508
#define OID_802_11_SSID			0x0509
#define OID_802_11_BSSID		0x050A
#define RT_OID_802_11_RADIO		0x050B
#define RT_OID_802_11_PHY_MODE		0x050C
#define OID_802_11_BSSID_LIST		0x0609
#define OID_802_3_CURRENT_ADDRESS	0x060A
#define OID_GEN_MEDIA_CONNECT_STATUS	0x060B
#define RT_OID_GET_PHY_MODE		0x0761
#define OID_GET_SET_TOGGLE		0x8000
#define RT_OID_SYNC_RT61		0x0D010750
#define RT_OID_WSC_QUERY_STATUS		((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE		((RT_OID_SYNC_RT61 + 0x02) & 0xffff)

#define MTD_PART_NAME_FACTORY	"Factory"
#define MTD_PART_NAME_KERNEL	"Kernel"
#define MTD_PART_NAME_RWFS	"RWFS"

#if defined (BOARD_360P2)
#define OFFSET_MAC_GMAC0	0xFFE8
#define OFFSET_MAC_GMAC2	0xFFEE
#elif defined (CONFIG_RALINK_MT7621)
#define OFFSET_MAC_GMAC0	0xE000
#define OFFSET_MAC_GMAC2	0xE006
#else
#define OFFSET_MAC_GMAC0	0x0028
#define OFFSET_MAC_GMAC2	0x0022
#endif
#define OFFSET_MAC_ADDR_WSOC	0x0004
#define OFFSET_MAC_ADDR_INIC	0x8004
#define OFFSET_PIN_CODE		0x0180
#define OFFSET_TXBF_PARA	0x01A0
#define OFFSET_BOOT_VER		0x018A
#define OFFSET_COUNTRY_CODE	0x0188
#define OFFSET_REGSPEC_CODE	0x0248

#endif /* _ralink_h_ */
