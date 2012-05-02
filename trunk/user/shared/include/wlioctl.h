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
 * Custom OID/ioctl definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
 *
 * Copyright 2001-2003, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wlioctl.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 */

#ifndef _wlioctl_h_
#define	_wlioctl_h_

#include <nvram/typedefs.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>

#if defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#define	PACKED
#endif

/*
 * Per-bss information structure.
 */

#define WL_NUMRATES		255	/* max # of rates in a rateset */

//typedef unsigned char   bool;   // 1204 ham

typedef struct wl_rateset {
	uint32	count;			/* # rates in this set */
	uint8	rates[WL_NUMRATES];	/* rates in 500kbps units w/hi bit set if basic */
} wl_rateset_t;

#define	WL_LEGACY_BSS_INFO_VERSION	106	/* an older supported version of wl_bss_info struct */
#define	WL_BSS_INFO_VERSION		107	/* current version of wl_bss_info struct */

typedef struct wl_bss_info106 {
	uint		version;	/* version field */
	struct ether_addr BSSID;
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		Privacy;	/* 0=No WEP, 1=Use WEP */
	int16		RSSI;		/* receive signal strength (in dBm) */
	uint16		beacon_period;	/* units are Kusec */
	uint16		atim_window;	/* units are Kusec */
	uint8		channel;	/* Channel no. */
	int8		infra;		/* 0=IBSS, 1=infrastructure, 2=unknown */
	struct {
		uint	count;		/* # rates in this set */
		uint8	rates[12];	/* rates in 500kbps units w/hi bit set if basic */
	} rateset;			/* supported rates */
        uint8           dtim_period;    /* DTIM period */
	int8		phy_noise;	/* noise right after tx (in dBm) */
	uint16		capability;	/* Capability information */
	struct dot11_bcn_prb *prb;	/* probe response frame (ioctl na) */
	uint16		prb_len;	/* probe response frame length (ioctl na) */
	struct {
		uint8 supported;	/* wpa supported */
		uint8 multicast;	/* multicast cipher */
		uint8 ucount;		/* count of unicast ciphers */
		uint8 unicast[4];	/* unicast ciphers */
		uint8 acount;		/* count of auth modes */
		uint8 auth[4];		/* Authentication modes */
	} wpa;
} wl_bss_info106_t;

typedef struct wl_bss_info {
	uint32		version;	/* version field */
	uint32		length;		/* byte length of data in this record, starting at version and including IEs */
	struct ether_addr BSSID;
	uint16		beacon_period;	/* units are Kusec */
	uint16		capability;	/* Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	struct {
		uint	count;		/* # rates in this set */
		uint8	rates[16];	/* rates in 500kbps units w/hi bit set if basic */
	} rateset;			/* supported rates */
	uint8		channel;	/* Channel no. */
	uint16		atim_window;	/* units are Kusec */
        uint8           dtim_period;    /* DTIM period */
	int16		RSSI;		/* receive signal strength (in dBm) */
	int8		phy_noise;	/* noise (in dBm) */
	uint32		ie_length;	/* byte length of Information Elements */
	/* variable length Information Elements */
} wl_bss_info_t;

typedef struct wl_scan_results {
	uint32 buflen;
	uint32 version;
	uint32 count;
	wl_bss_info_t bss_info[1];
} wl_scan_results_t;
/* size of wl_scan_results not including variable length array */
#define WL_SCAN_RESULTS_FIXED_SIZE 12

/* uint32 list */
typedef struct wl_uint32_list {
	/* in - # of elements, out - # of entries */
	uint32 count;
	/* variable length uint32 list */
	uint32 element[1];
} wl_uint32_list_t;

typedef struct wlc_ssid {
	uint32		SSID_len;
	uchar		SSID[32];
} wlc_ssid_t;

#define WLC_CNTRY_BUF_SZ        4       /* Country string is 3 bytes + NULL */

typedef struct wl_channels_in_country {
	uint32 buflen;
	uint32 band;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	uint32 count;
	uint32 channel[1];
} wl_channels_in_country_t;

typedef struct wl_country_list {
	uint32 buflen;
	uint32 band_set;
	uint32 band;
	uint32 count;
	char country_abbrev[1];
} wl_country_list_t;


/*
* Maximum # of keys that wl driver supports in S/W. Keys supported 
* in H/W is less than or equal to WSEC_MAX_KEYS.
*/
#define WSEC_MAX_KEYS		54	/* Max # of keys (50 + 4 default keys) */
#define WSEC_MAX_DEFAULT_KEYS	4	/* # of default keys */

/*
* Remove these two defines if access to crypto/tkhash.h 
* is unconditionally permitted.
*/
#define TKHASH_P1_KEY_SIZE	10	/* size of TKHash Phase1 output, in bytes */
#define TKHASH_P2_KEY_SIZE	16	/* size of TKHash Phase2 output */

/* Enumerate crypto algorithms */
#define	CRYPTO_ALGO_OFF			0
#define	CRYPTO_ALGO_WEP1		1
#define	CRYPTO_ALGO_TKIP		2
#define	CRYPTO_ALGO_WEP128		3
#define CRYPTO_ALGO_AES_CCM		4
#define CRYPTO_ALGO_AES_OCB_MSDU	5
#define CRYPTO_ALGO_AES_OCB_MPDU	6
#define CRYPTO_ALGO_NALG		7

/* For use with wlc_wep_key.flags */
#define WSEC_PRIMARY_KEY	(1 << 1)	/* Indicates this key is the primary (ie tx) key */
#define WSEC_TKIP_ERROR		(1 << 2)	/* Provoke deliberate MIC error */
#define WSEC_REPLAY_ERROR	(1 << 3)	/* Provoke deliberate replay */

#define WSEC_GEN_MIC_ERROR	0x0001
#define WSEC_GEN_REPLAY		0x0002

typedef struct tkip_info {
	uint16		phase1[TKHASH_P1_KEY_SIZE/sizeof(uint16)];	/* tkhash phase1 result */
	uint8		phase2[TKHASH_P2_KEY_SIZE];	/* tkhash phase2 result */
	uint32		micl;
	uint32		micr;
} tkip_info_t;

typedef struct wsec_iv {
	uint32		hi;	/* upper 32 bits of IV */
	uint16		lo;	/* lower 16 bits of IV */
} wsec_iv_t;

typedef struct wsec_key {
	uint32		index;		/* key index */
	uint32		len;		/* key length */
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	tkip_info_t	tkip_tx;	/* tkip transmit state */
	tkip_info_t	tkip_rx;	/* tkip receive state */
	uint32		algo;		/* CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint32		flags;		/* misc flags */
	uint32 		algo_hw;	/* cache for hw register*/
	uint32 		aes_mode;	/* cache for hw register*/
	int		iv_len;		/* IV length */		
	int		iv_initialized;	/* has IV been initialized already? */		
	int		icv_len;	/* ICV length */
	wsec_iv_t	rxiv;		/* Rx IV */
	wsec_iv_t	txiv;		/* Tx IV */
	struct ether_addr ea;		/* per station */
} wsec_key_t;

/* wireless security bitvec */
#define WEP_ENABLED		1
#define TKIP_ENABLED		2
#define AES_ENABLED		4
#define WSEC_SWFLAG		8

#define WSEC_SW(wsec)		((wsec) & WSEC_SWFLAG)
#define WSEC_HW(wsec)		(!WSEC_SW(wsec))
#define WSEC_WEP_ENABLED(wsec)	((wsec) & WEP_ENABLED)
#define WSEC_TKIP_ENABLED(wsec)	((wsec) & TKIP_ENABLED)
#define WSEC_AES_ENABLED(wsec)	((wsec) & AES_ENABLED)
#define WSEC_ENABLED(wsec)	((wsec) & (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED))

/* wireless authentication bit vector */
#define WPA_ENABLED	1
#define PSK_ENABLED	2

#define WAUTH_WPA_ENABLED(wauth)	((wauth) & WPA_ENABLED)
#define WAUTH_PSK_ENABLED(wauth)	((wauth) & PSK_ENABLED)
#define WAUTH_ENABLED(wauth)		((wauth) & (WPA_ENABLED | PSK_ENABLED))

/* group/mcast cipher */
#define WPA_MCAST_CIPHER(wsec)	(((wsec) & TKIP_ENABLED) ? WPA_CIPHER_TKIP : \
				((wsec) & AES_ENABLED) ? WPA_CIPHER_AES_CCM : \
				WPA_CIPHER_NONE)

typedef struct wl_led_info {
	uint32		index;		/* led index */
	uint32		behavior;
	bool		activehi;
} wl_led_info_t;

/*
 * definitions for driver messages passed from WL to NAS.
 */
/* Use this to recognize wpa and 802.1x driver messages. */
static const uint8 wl_wpa_snap_template[] =
	{ 0xaa, 0xaa, 0x03, 0x00, 0x90, 0x4c };

#define WL_WPA_MSG_IFNAME_MAX	16

/* WPA driver message */
typedef struct wl_wpa_header {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 version;
	uint8 type;
	/* version 2 additions */
	char ifname[WL_WPA_MSG_IFNAME_MAX];
	/* version specific data */
	/* uint8 data[1]; */
} wl_wpa_header_t PACKED;

#define WL_WPA_HEADER_LEN	(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + 2 + WL_WPA_MSG_IFNAME_MAX)

/* WPA driver message ethertype - private between wlc and nas */
#define WL_WPA_ETHER_TYPE	0x9999

/* WPA driver message current version */
#define WL_WPA_MSG_VERSION	2

/* Type field values for the 802.2 driver messages for WPA. */
#define WLC_ASSOC_MSG		1
#define WLC_DISASSOC_MSG	2
#define WLC_PTK_MIC_MSG		3
#define WLC_GTK_MIC_MSG		4

/* 802.1x driver message */
typedef struct wl_eapol_header {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 version;
	uint8 reserved;
	char ifname[WL_WPA_MSG_IFNAME_MAX];
	/* version specific data */
	/* uint8 802_1x_msg[1]; */
} wl_eapol_header_t PACKED;

#define WL_EAPOL_HEADER_LEN	(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + 2 + WL_WPA_MSG_IFNAME_MAX)

/* 802.1x driver message ethertype - private between wlc and nas */
#define WL_EAPOL_ETHER_TYPE	0x999A

/* 802.1x driver message current version */
#define WL_EAPOL_MSG_VERSION	1

/* srom read/write struct passed through ioctl */
typedef struct {
	uint   byteoff;		/* byte offset */
	uint   nbytes;		/* number of bytes */
	uint16 buf[1];
} srom_rw_t;

/* R_REG and W_REG struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/* byte offset of the field in d11regs_t */
	uint32	val;		/* read/write value of the field */
	uint32	size;		/* sizeof the field */
} rw_reg_t;

/* Structure used by GET/SET_ATTEN ioctls */
typedef struct {
	uint16	auto_ctrl;	/* 1: Automatic control, 0: overriden */
	uint16	bb;		/* Baseband attenuation */
	uint16	radio;		/* Radio attenuation */
	uint16	txctl1;		/* Radio TX_CTL1 value */
} atten_t;

/* Used to get specific STA parameters */ 
typedef struct {
	uint32	val;
	struct ether_addr ea;
} scb_val_t;

/* callback registration data types */

typedef struct _mac_event_params {
	uint msg;
	struct ether_addr *addr;
	uint result;
	uint status; 
	uint auth_type;
} mac_event_params_t;

typedef struct _mic_error_params {
	struct ether_addr *ea;
	bool group;
	bool flush_txq;
} mic_error_params_t;

typedef enum _wl_callback {
	WL_MAC_EVENT_CALLBACK = 0,
	WL_LINK_UP_CALLBACK,
	WL_LINK_DOWN_CALLBACK,
	WL_MIC_ERROR_CALLBACK,
	WL_LAST_CALLBACK
} wl_callback_t;

typedef struct _callback {
	void (*fn)(void *, void *);
	void *context;
} callback_t;

typedef struct _scan_callback {
	void (*fn)(void *);
	void *context;
} scan_callback_t;

/* used to register an arbitrary callback via the IOCTL interface */
typedef struct _set_callback {
	int index;
	callback_t callback;
} set_callback_t;

/*
 * Country locale determines which channels are available to us.
 */
typedef enum _wlc_locale {
	WLC_WW = 0,	/* Worldwide */
	WLC_THA,	/* Thailand */
	WLC_ISR,	/* Israel */
	WLC_JDN,	/* Jordan */
	WLC_PRC,	/* China */
	WLC_JPN,	/* Japan */
	WLC_FCC,	/* USA */
	WLC_EUR,	/* Europe */
	WLC_USL,	/* US Low Band only */
	WLC_JPH,	/* Japan High Band only */
	WLC_ALL,	/* All the channels in this band */
	WLC_11D,	/* Represents locale recieved by 11d beacons */
	WLC_LAST_LOCALE,
	WLC_UNDEFINED_LOCALE = 0xf
} wlc_locale_t;

/* channel encoding */
typedef struct channel_info {
	int hw_channel;
	int target_channel;
	int scan_channel;
} channel_info_t;

/* For ioctls that take a list of MAC addresses */
struct maclist {
	uint count;			/* number of MAC addresses */
	struct ether_addr ea[1];	/* variable length array of MAC addresses */
};

/* get pkt count struct passed through ioctl */
typedef struct get_pktcnt {
	uint rx_good_pkt;
	uint rx_bad_pkt;
	uint tx_good_pkt;
	uint tx_bad_pkt;
} get_pktcnt_t;

/* Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	int cmd;	/* common ioctl definition */
	void *buf;	/* pointer to user buffer */
	int len;	/* length of user buffer */
} wl_ioctl_t;

/* 
 * Structure for passing hardware and software 
 * revision info up from the driver. 
 */
typedef struct wlc_rev_info {
	uint		vendorid;	/* PCI vendor id */
	uint		deviceid;	/* device id of chip */
	uint		radiorev;	/* radio revision */
	uint		chiprev;	/* chip revision */
	uint		corerev;	/* core revision */
	uint		boardid;	/* board identifier (usu. PCI sub-device id) */
	uint		boardvendor;	/* board vendor (usu. PCI sub-vendor id) */
	uint		boardrev;	/* board revision */
	uint		driverrev;	/* driver version */
	uint		ucoderev;	/* microcode version */
	uint		bus;		/* bus type */
	uint        chipnum;    /* chip number */
} wlc_rev_info_t;

/* check this magic number */
#define WLC_IOCTL_MAGIC		0x14e46c77

/* bump this number if you change the ioctl interface */
#define WLC_IOCTL_VERSION	1

/* maximum length buffer required */
#define WLC_IOCTL_MAXLEN	8192

/* common ioctl definitions */
#define WLC_GET_MAGIC				0
#define WLC_GET_VERSION				1
#define WLC_UP					2
#define WLC_DOWN				3
#define WLC_DUMP				6
#define WLC_GET_MSGLEVEL			7
#define WLC_SET_MSGLEVEL			8
#define WLC_GET_PROMISC				9
#define WLC_SET_PROMISC				10
#define WLC_GET_RATE				12
#define WLC_SET_RATE				13
#define WLC_GET_INSTANCE			14
#define WLC_GET_FRAG				15
#define WLC_SET_FRAG				16
#define WLC_GET_RTS				17
#define WLC_SET_RTS				18
#define WLC_GET_INFRA				19
#define WLC_SET_INFRA				20
#define WLC_GET_AUTH				21
#define WLC_SET_AUTH				22
#define WLC_GET_BSSID				23
#define WLC_SET_BSSID				24
#define WLC_GET_SSID				25
#define WLC_SET_SSID				26
#define WLC_RESTART				27
#define WLC_GET_CHANNEL				29
#define WLC_SET_CHANNEL				30
#define WLC_GET_SRL				31
#define WLC_SET_SRL				32
#define WLC_GET_LRL				33
#define WLC_SET_LRL				34
#define WLC_GET_PLCPHDR				35
#define WLC_SET_PLCPHDR				36
#define WLC_GET_RADIO				37
#define WLC_SET_RADIO				38
#define WLC_GET_PHYTYPE				39
#define WLC_GET_WEP				42
#define WLC_SET_WEP				43
#define WLC_GET_KEY				44
#define WLC_SET_KEY				45
#define WLC_SCAN				50
#define WLC_SCAN_RESULTS			51
#define WLC_DISASSOC				52
#define WLC_REASSOC				53
#define WLC_GET_ROAM_TRIGGER			54
#define WLC_SET_ROAM_TRIGGER			55
#define WLC_GET_TXANT				61
#define WLC_SET_TXANT				62
#define WLC_GET_ANTDIV				63
#define WLC_SET_ANTDIV				64
#define WLC_GET_TXPWR				65
#define WLC_SET_TXPWR				66
#define WLC_GET_CLOSED				67
#define WLC_SET_CLOSED				68
#define WLC_GET_MACLIST				69
#define WLC_SET_MACLIST				70
#define WLC_GET_RATESET				71
#define WLC_SET_RATESET				72
#define WLC_GET_LOCALE				73
#define WLC_SET_LOCALE				74
#define WLC_GET_BCNPRD				75
#define WLC_SET_BCNPRD				76
#define WLC_GET_DTIMPRD				77
#define WLC_SET_DTIMPRD				78
#define WLC_GET_SROM				79
#define WLC_SET_SROM				80
#define WLC_GET_WEP_RESTRICT			81
#define WLC_SET_WEP_RESTRICT			82
#define WLC_GET_COUNTRY				83
#define WLC_SET_COUNTRY				84
#define WLC_GET_REVINFO				98
#define WLC_GET_MACMODE				105
#define WLC_SET_MACMODE				106
#define WLC_GET_GMODE				109
#define WLC_SET_GMODE				110
#define WLC_GET_CURR_RATESET			114	/* current rateset */
#define WLC_GET_SCANSUPPRESS			115
#define WLC_SET_SCANSUPPRESS			116
#define WLC_GET_AP				117
#define WLC_SET_AP				118
#define WLC_GET_EAP_RESTRICT			119
#define WLC_SET_EAP_RESTRICT			120
#define WLC_GET_WDSLIST				123
#define WLC_SET_WDSLIST				124
#define WLC_GET_RSSI				127
#define WLC_GET_WSEC				133
#define WLC_SET_WSEC				134
#define WLC_GET_BSS_INFO			136
#define WLC_GET_LAZYWDS				138
#define WLC_SET_LAZYWDS				139
#define WLC_GET_BANDLIST			140
#define WLC_GET_BAND				141
#define WLC_SET_BAND				142
#define WLC_GET_SHORTSLOT			144
#define WLC_GET_SHORTSLOT_OVERRIDE		145
#define WLC_SET_SHORTSLOT_OVERRIDE		146
#define WLC_GET_SHORTSLOT_RESTRICT		147
#define WLC_SET_SHORTSLOT_RESTRICT		148
#define WLC_GET_GMODE_PROTECTION		149
#define WLC_GET_GMODE_PROTECTION_OVERRIDE	150
#define WLC_SET_GMODE_PROTECTION_OVERRIDE	151
#define WLC_UPGRADE				152
#define WLC_GET_ASSOCLIST			159
#define WLC_GET_CLK				160
#define WLC_SET_CLK				161
#define WLC_GET_UP				162
#define WLC_OUT					163
#define WLC_GET_WPA_AUTH			164
#define WLC_SET_WPA_AUTH			165
#define WLC_GET_GMODE_PROTECTION_CONTROL	178
#define WLC_SET_GMODE_PROTECTION_CONTROL	179
#define WLC_GET_PHYLIST				180
#define WLC_GET_GMODE_PROTECTION_CTS		198
#define WLC_SET_GMODE_PROTECTION_CTS		199
#define WLC_GET_PIOMODE				203
#define WLC_SET_PIOMODE				204
#define WLC_SET_LED				209
#define WLC_GET_LED				210
#define WLC_GET_CHANNEL_SEL			215
#define WLC_START_CHANNEL_SEL			216
#define WLC_GET_VALID_CHANNELS			217
#define WLC_GET_FAKEFRAG			218
#define WLC_SET_FAKEFRAG			219
#define WLC_GET_WET				230
#define WLC_SET_WET				231
#define WLC_GET_KEY_PRIMARY			235
#define WLC_SET_KEY_PRIMARY			236
#define WLC_SCAN_WITH_CALLBACK			240
#define WLC_SET_CS_SCAN_TIMER			248
#define WLC_GET_CS_SCAN_TIMER			249
#define WLC_CURRENT_PWR				256
#define WLC_GET_CHANNELS_IN_COUNTRY		260
#define WLC_GET_COUNTRY_LIST			261
#define	WLC_GET_VAR				262	/* get value of named variable */
#define	WLC_SET_VAR				263	/* set named variable to value */
#define WLC_NVRAM_GET				264
#define WLC_NVRAM_SET				265
#define WLC_LAST				271	/* bump after adding */

/*
 * Minor kludge alert:
 * Duplicate a few definitions that irelay requires from epiioctl.h here
 * so caller doesn't have to include this file and epiioctl.h .
 * If this grows any more, it would be time to move these irelay-specific
 * definitions out of the epiioctl.h and into a separate driver common file.
 */
#ifndef EPICTRL_COOKIE
#define EPICTRL_COOKIE		0xABADCEDE
#endif

/* vx wlc ioctl's offset */
#define CMN_IOCTL_OFF 0x180

/*
 * custom OID support
 *
 * 0xFF - implementation specific OID
 * 0xE4 - first byte of Broadcom PCI vendor ID
 * 0x14 - second byte of Broadcom PCI vendor ID
 * 0xXX - the custom OID number
 */

/* begin 0x1f values beyond the start of the ET driver range. */
#define WL_OID_BASE		0xFFE41420

/* NDIS overrides */
#define OID_WL_GETINSTANCE	(WL_OID_BASE + WLC_GET_INSTANCE)

#define WL_DECRYPT_STATUS_SUCCESS	1
#define WL_DECRYPT_STATUS_FAILURE	2
#define WL_DECRYPT_STATUS_UNKNOWN	3

/* allows user-mode app to poll the status of USB image upgrade */
#define WLC_UPGRADE_SUCCESS			0
#define WLC_UPGRADE_PENDING			1

/* Bit masks for radio disabled status - returned by WL_GET_RADIO */
#define WL_RADIO_SW_DISABLE	(1<<0)
#define WL_RADIO_HW_DISABLE	(1<<1)

/* Override bit for WLC_SET_TXPWR.  if set, ignore other level limits */
#define WL_TXPWR_OVERRIDE	(1<<31)


/* Bus types */
#define WL_SB_BUS	0	/* Silicon Backplane */
#define WL_PCI_BUS	1	/* PCI target */
#define WL_PCMCIA_BUS	2	/* PCMCIA target */

/* band types */
#define	WLC_BAND_AUTO		0	/* auto-select */
#define	WLC_BAND_A		1	/* "a" band (5   Ghz) */
#define	WLC_BAND_B		2	/* "b" band (2.4 Ghz) */

/* band types */
#define WLC_BAND_5G             1       /* 5 Ghz */
#define WLC_BAND_2G             2       /* 2.4 Ghz */
#define WLC_BAND_ALL            3       /* all bands */


/* MAC list modes */
#define WLC_MACMODE_DISABLED	0	/* MAC list disabled */
#define WLC_MACMODE_DENY	1	/* Deny specified (i.e. allow unspecified) */
#define WLC_MACMODE_ALLOW	2	/* Allow specified (i.e. deny unspecified) */	

/* 
 *
 */
#define GMODE_LEGACY_B		0
#define GMODE_AUTO		1
#define GMODE_ONLY		2
#define GMODE_B_DEFERRED	3
#define GMODE_PERFORMANCE	4
#define GMODE_LRS		5
#define GMODE_AFTERBURNER	6
#define GMODE_MAX		7

/* values for PLCPHdr_override */
#define WLC_PLCP_AUTO	-1
#define WLC_PLCP_SHORT	0
#define WLC_PLCP_LONG	1

/* values for g_protection_override */
#define WLC_G_PROTECTION_AUTO	-1
#define WLC_G_PROTECTION_OFF	0
#define WLC_G_PROTECTION_ON	1

/* values for g_protection_control */
#define WLC_G_PROTECTION_CTL_OFF	0
#define WLC_G_PROTECTION_CTL_LOCAL	1
#define WLC_G_PROTECTION_CTL_OVERLAP	2






/* max # of leds supported by GPIO (gpio pin# == led index#) */
#define	WL_LED_NUMGPIO		16	/* gpio 0-15 */

/* led per-pin behaviors */
#define	WL_LED_OFF		0		/* always off */
#define	WL_LED_ON		1		/* always on */
#define	WL_LED_ACTIVITY		2		/* activity */
#define	WL_LED_RADIO		3		/* radio enabled */
#define	WL_LED_ARADIO		4		/* 5  Ghz radio enabled */
#define	WL_LED_BRADIO		5		/* 2.4Ghz radio enabled */
#define	WL_LED_BGMODE		6		/* on if gmode, off if bmode */
#define	WL_LED_WI1		7		
#define	WL_LED_WI2		8		
#define	WL_LED_WI3		9		
#define	WL_LED_ASSOC		10		/* associated state indicator */
#define	WL_LED_INACTIVE		11		/* null behavior (clears default behavior) */
#define	WL_LED_NUMBEHAVIOR	12

/* led behavior numeric value format */
#define	WL_LED_BEH_MASK		0x7f		/* behavior mask */
#define	WL_LED_AL_MASK		0x80		/* activelow (polarity) bit */


/* rate check */
#define WL_RATE_OFDM(r)		(((r) & 0x7f) == 12 || ((r) & 0x7f) == 18 || \
				 ((r) & 0x7f) == 24 || ((r) & 0x7f) == 36 || \
				 ((r) & 0x7f) == 48 || ((r) & 0x7f) == 72 || \
				 ((r) & 0x7f) == 96 || ((r) & 0x7f) == 108)


#undef PACKED

#endif /* _wlioctl_h_ */
