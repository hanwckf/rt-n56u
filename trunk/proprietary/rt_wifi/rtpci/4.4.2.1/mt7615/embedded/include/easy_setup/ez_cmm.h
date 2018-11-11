#ifndef __EZ_CMM_H__
#define __EZ_CMM_H__

#ifdef WH_EZ_SETUP

#ifdef DOT11R_FT_SUPPORT
#include "dot11r_ft.h"
#endif

#define IS_EZ_SETUP_ENABLED(_wdev)  (((_wdev)->enable_easy_setup) == TRUE)
#define IS_ADPTR_EZ_SETUP_ENABLED(pAd) (pAd->ApCfg.MBSSID[0].wdev.enable_easy_setup)

#define MAX_EZ_BANDS 						2
#define MAX_NON_EZ_BANDS 						2

#define MOBILE_APP_SUPPORT 			1
#define DEDICATED_MAN_AP   			1
//! Levarage from CL170210
#define CONFIG_PUSH_VER_SUPPORT 	1

typedef struct __ez_band_info{
	void *  pAd;
	CHAR func_idx; 
	struct wifi_dev *ez_ap_wdev;
	struct wifi_dev *ez_cli_wdev;
} EZ_BAND_INFO;



#ifdef NEW_CONNECTION_ALGO
enum ACTION_TYPE{
	ACTION_TYPE_NONE =0,
	ACTION_TYPE_DELAY_DISCONNECT =1,
	ACTION_TYPE_UPDATE_CONFIG = 2,
	ACTION_TYPE_NOTIFY_ROAM = 3
};
#endif

#ifdef DBG
extern unsigned long ez_memory_alloc_num;
extern unsigned long ez_memory_free_num;
#endif /* DBG */

#ifdef EZ_DUAL_BAND_SUPPORT
#define IS_SINGLE_CHIP_DBDC(_pAd)	((PRTMP_ADAPTER)_pAd)->SingleChip
#define IS_DUAL_CHIP_DBDC(_pAd)		!(IS_SINGLE_CHIP_DBDC(_pAd))
#else
#define IS_SINGLE_CHIP_DBDC(_pAd)	1
#define IS_DUAL_CHIP_DBDC(_pAd)		0
#endif


#define MTK_VENDOR_CAPABILITY_SIZE    4
#define MTK_VENDOR_EASY_SETUP         0x40
#define MTK_OUI_LEN                   3
#define RALINK_OUI_LEN                3

#define AUTH_MODE_EZ                  0xFF01

#define EZ_DH_KEY_LEN                 32
#define EZ_NONCE_LEN                  32
#define EZ_RAW_KEY_LEN                192
#define EZ_MIC_LEN                    16
#define EZ_PMK_LEN                    32
#define EZ_PTK_LEN                    80
#define EZ_GTK_LEN                    32
#define EZ_CAPABILITY_LEN             4
#define EZ_GROUP_ID_LEN               4

#define EZ_TAG_SDH_PUBLIC_KEY         0x01
#define EZ_TAG_SNONCE                 0x02
#define EZ_TAG_ADH_PUBLIC_KEY         0x03
#define EZ_TAG_ANONCE                 0x04
#define EZ_TAG_GTK                    0x05
#define EZ_TAG_CAPABILITY_INFO        0x06
#define EZ_TAG_MIC                    0x07
#define EZ_TAG_GROUP_ID               0x08
#define EZ_TAG_PMK                    0x09
#define EZ_TAG_APCLI_ACTION_INFO      0x0a
#ifdef EZ_NETWORK_MERGE_SUPPORT
//#define EZ_TAG_SSID					  0x0b
#define EZ_TAG_NETWORK_WEIGHT		  0x0c	
#define EZ_TAG_OTHER_BAND_PMK		  0x0d
//#define EZ_TAG_OTHER_BAND_SSID		  0x0e
#define EZ_TAG_GROUP_ID_UPDATE        0x0f
//#define EZ_TAG_TARGET_CHANNEL         0x10
#define EZ_TAG_DELAY_DISCONNECT_COUNT 0x11
#define EZ_TAG_DEVICE_INFO            0x13
#ifdef EZ_DUAL_BAND_SUPPORT
#define EZ_TAG_INTERFACE_INFO         0x12
#endif
#define EZ_TAG_BEACON_INFO         	  0x14
#define EZ_TAG_NOTIFY_ROAM			  0x15
#endif
#define EZ_TAG_NON_EZ_BEACON       	  0x16

#define EZ_TAG_TRIBAND_SEC       	  0x17

#ifdef NEW_CONNECTION_ALGO
#define EZ_TAG_NODE_NUMBER			  0x20
#endif
#ifdef NEW_CONNECTION_ALGO
#define EZ_TAG_AP_MAC		  0x21
#define EZ_TAG_OPEN_GROUP_ID	0x22
#endif

#define EZ_TAG_NON_EZ_CONFIG	0x23
#define EZ_TAG_NON_EZ_PSK		0x24
#define EZ_TAG_COUSTOM_DATA		0x25
#define EZ_TAG_NON_MAN_CONFIG   0x26


#define EZ_TLV_TAG_SIZE               1
#define EZ_TLV_LEN_SIZE               1

#define EZ_TAG_OFFSET                 (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE)
#define EZ_TAG_LEN_OFFSET             (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE+EZ_TLV_TAG_SIZE)
#define EZ_TAG_DATA_OFFSET            (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE+EZ_TLV_TAG_SIZE+EZ_TLV_LEN_SIZE)

#define EZ_BEACON_TAG_COUNT           1
#ifdef EZ_NETWORK_MERGE_SUPPORT
#define EZ_PROB_REQ_TAG_COUNT         2 //! aditional, capabilities tag required in preobe-response
#define EZ_UDATE_CONFIG_TAG_COUNT     7
#else 
#define EZ_PROB_REQ_TAG_COUNT         1
#endif
#define EZ_PROB_RSP_TAG_COUNT         2
#define EZ_AUTH_REQ_TAG_COUNT         4
#define EZ_AUTH_RSP_TAG_COUNT         4
#ifdef NEW_CONNECTION_ALGO
#define EZ_ASSOC_REQ_TAG_COUNT        4
#else
#define EZ_ASSOC_REQ_TAG_COUNT        1
#endif

#if defined (NEW_CONNECTION_ALGO) && defined (EZ_NETWORK_MERGE_SUPPORT)
#define EZ_ASSOC_RSP_TAG_COUNT        5//! other band PMK, SSID and network weight
#elif defined (EZ_NETWORK_MERGE_SUPPORT)
#define EZ_ASSOC_RSP_TAG_COUNT        5 //! other band PMK, SSID and network weight
#elif defined (NEW_CONNECTION_ALGO)
#define EZ_ASSOC_RSP_TAG_COUNT        3
#else
#define EZ_ASSOC_RSP_TAG_COUNT        2
#endif


#define EZ_STATUS_CODE_SUCCESS         0x0000
#define EZ_STATUS_CODE_MIC_ERROR       0x0001
#define EZ_STATUS_CODE_INVALID_DATA    0x0002
#define EZ_STATUS_CODE_NO_RESOURCE     0x0003
#define EZ_STATUS_CODE_PEER_CONNECTED  0x0004
#define EZ_STATUS_CODE_LOOP			   0x0005
#define EZ_STATUS_CODE_UNKNOWN         0xFFFF

#define EZ_MAX_STA_NUM 8

#define EZ_AES_KEY_ENCRYPTION_EXTEND	8 /* 8B for AES encryption extend size */

#define EZ_PAUSE_SCAN_TIME_OUT          1000 /* 1 second */
#define EZ_MAX_SCAN_TIME_OUT            10000 /* 10 seconds */
#define EZ_STOP_SCAN_TIME_OUT           120000 /* 120 seconds */
#ifdef EZ_NETWORK_MERGE_SUPPORT
#define EZ_GROUP_MERGE_TIMEOUT          120000 /* 120 seconds */
#endif
#ifdef EZ_DUAL_BAND_SUPPORT
#define EZ_LOOP_CHK_TIMEOUT_10S          	10000 /* 10 seconds */
#define EZ_LOOP_CHK_TIMEOUT_5S          	5000 /* 5 seconds */
#endif

#define EZ_UNCONFIGURED                 1
#define EZ_CONFIGURED                   2

#define EZ_INDEX_NOT_FOUND              0xFF

#define EZ_CAP_CONFIGURED 			(1 << 0)
#define EZ_CAP_INTERNET   			(1 << 1)
#define EZ_CAP_MEMBER_COUNT(__x) 	(__x << 2)
#define EZ_CAP_HAS_APCLI_INF  		(1 << 10)
#define EZ_CAP_CONNECTED  			(1 << 11)
#ifdef  EZ_NETWORK_MERGE_SUPPORT
#define EZ_CAP_ALLOW_MERGE  		(1 << 12)
#define EZ_CAP_AP_CONFIGURED        (1 << 13)
#endif
#define EZ_SET_CAP_CONFIGRED(__cap) (__cap |= EZ_CAP_CONFIGURED)
#define EZ_SET_CAP_INTERNET(__cap) (__cap |= EZ_CAP_INTERNET)
#define EZ_SET_CAP_MEMBER_COUNT(__cap, __y) (__cap |= EZ_CAP_MEMBER_COUNT(__y))
#define EZ_SET_CAP_HAS_APCLI_INF(__cap) (__cap |= EZ_CAP_HAS_APCLI_INF)
#define EZ_SET_CAP_CONNECTED(__cap) (__cap |= EZ_CAP_CONNECTED)
#ifdef EZ_NETWORK_MERGE_SUPPORT
#define EZ_SET_CAP_ALLOW_MERGE(__cap) (__cap |= EZ_CAP_ALLOW_MERGE)
#endif
#ifdef NEW_CONNECTION_ALGO
#define EZ_SET_CAP_AP_CONFIGURED(__cap) (__cap |= EZ_CAP_AP_CONFIGURED)
#endif

#define EZ_CLEAR_CAP_CONFIGRED(__cap) (__cap &= 0xFFFFFFFE)
#define EZ_CLEAR_CAP_INTERNET(__cap) (__cap &= 0xFFFFFFFD)
#define EZ_CLEAR_CAP_MEMBER_COUNT(__cap) (__cap &= 0xFFFFFC03)
#define EZ_CLEAR_CAP_HAS_APCLI_INF(__cap) (__cap &= 0xFFFFFBFF)
#define EZ_CLEAR_CAP_CONNECTED(__cap) (__cap &= 0xFFFFF7FF)
#ifdef EZ_NETWORK_MERGE_SUPPORT
#define EZ_CLEAR_CAP_ALLOW_MERGE(__cap) (__cap &= 0xFFFFEFFF)
#endif
#ifdef NEW_CONNECTION_ALGO
#define EZ_CLEAR_CAP_AP_CONFIGURED(__cap) (__cap &= 0xFFFFEFFF)
#endif

#define EZ_GET_CAP_CONFIGRED(__cap) (__cap & EZ_CAP_CONFIGURED)
#define EZ_GET_CAP_INTERNET(__cap) (__cap & EZ_CAP_INTERNET)
#define EZ_GET_CAP_HAS_APCLI_INF(__cap) (__cap & EZ_CAP_HAS_APCLI_INF)
#define EZ_GET_CAP_CONNECTED(__cap) (__cap & EZ_CAP_CONNECTED)
#ifdef EZ_NETWORK_MERGE_SUPPORT
#define EZ_GET_CAP_ALLOW_MERGE(__cap) (__cap & EZ_CAP_ALLOW_MERGE)
#endif
#ifdef NEW_CONNECTION_ALGO
#define EZ_GET_CAP_AP_CONFIGURED(__cap) (__cap & EZ_CAP_AP_CONFIGURED)
#endif

#define EZ_CLEAR_ACTION 0
#define EZ_SET_ACTION 1
#define EZ_UPDATE_CAPABILITY_INFO(__ad, __action, __cap_item, __inf_idx) \
{ \
	do { \
		if (!(__ad->ApCfg.MBSSID[__inf_idx].wdev.enable_easy_setup)) \
			break; \
		if (__action == EZ_SET_ACTION) \
			EZ_SET_CAP_ ## __cap_item((__ad->ApCfg.MBSSID[__inf_idx].wdev.ez_security.capability)); \
		else \
			EZ_CLEAR_CAP_ ## __cap_item((__ad->ApCfg.MBSSID[__inf_idx].wdev.ez_security.capability)); \
		UpdateBeaconHandler(__ad, &(__ad->ApCfg.MBSSID[__inf_idx].wdev), IE_CHANGE); \
	} while(0); \
}
#define EZ_UPDATE_APCLI_CAPABILITY_INFO(__ad, __action, __cap_item, __inf_idx) \
{ \
	do { \
		if (!(__ad->ApCfg.ApCliTab[__inf_idx].wdev.enable_easy_setup)) \
			break; \
		if (__action == EZ_SET_ACTION) \
			EZ_SET_CAP_ ## __cap_item((__ad->ApCfg.ApCliTab[__inf_idx].wdev.ez_security.capability)); \
		else \
			EZ_CLEAR_CAP_ ## __cap_item((__ad->ApCfg.ApCliTab[__inf_idx].wdev.ez_security.capability)); \
	} while(0); \
}
extern int EzDebugLevel;

#ifdef OLD_LOG
#define DBG_CAT_ALL     0xFFFFFFFFu
#define DBG_SUBCAT_ALL	0xFFFFFFFFu

/* Debug Level */
#define DBG_LVL_OFF     0
#define DBG_LVL_ERROR   1
#define DBG_LVL_WARN    2
#define DBG_LVL_TRACE   3
#define DBG_LVL_INFO    4
#define DBG_LVL_LOUD    5
#define DBG_LVL_NOISY   6
#define DBG_LVL_MAX     DBG_LVL_NOISY

#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
	DBGPRINT(__debug_level, __fmt);
#else
#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
do{ \
	if (__debug_level <= EzDebugLevel) { \
			printk __fmt;\
	}	\
}while(0)
#endif

#ifdef OLD_PRJ
#define MEDIATEK_EASY_SETUP (1 << 6)
typedef struct GNU_PACKED _ie_hdr {
    UCHAR eid;
    UINT8 len;
} IE_HEADER;

struct GNU_PACKED _mediatek_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR cap0;
    UCHAR cap1;
    UCHAR cap2;
    UCHAR cap3;
};

#ifdef DBG
#define EZ_MEM_ALLOC(__ad, __ptr, __size) \
	os_alloc_mem(__ad, (UCHAR **)(__ptr), __size); \
	ez_memory_alloc_num++;
#define EZ_MEM_FREE(__ad, __ptr) \
	MlmeFreeMemory(__ad, __ptr); \
	ez_memory_free_num++;
#else /* DBG */
#define EZ_MEM_ALLOC(__ad, __ptr, __size) \
	os_alloc_mem(__ad, (UCHAR **)(__ptr), __size);
#define EZ_MEM_FREE(__ad, __ptr) \
	MlmeFreeMemory(__ad, __ptr);
#endif /* !DBG */

#define EZ_DERIVE_PTK(__ad, __pmk, __a_nonce, __a_addr, __s_nonce, __s_addr, __ptk, __len) \
	WpaDerivePTK(__ad, __pmk, __a_nonce, __a_addr, __s_nonce, __s_addr, __ptk, __len);

#define EZ_GET_ENTRY_PTK(__entry) \
	&__entry->PTK[0]
	
#else

#ifdef DBG
#define EZ_MEM_ALLOC(__ad, __ptr, __size) \
	os_alloc_mem(__ad, (UCHAR **)(__ptr), __size); \
	ez_memory_alloc_num++;
#define EZ_MEM_FREE(__ad, __ptr) \
	MlmeFreeMemory(__ptr); \
	ez_memory_free_num++;
#else /* DBG */
#define EZ_MEM_ALLOC(__ad, __ptr, __size) \
	os_alloc_mem(__ad, (UCHAR **)(__ptr), __size);
#define EZ_MEM_FREE(__ad, __ptr) \
	MlmeFreeMemory(__ptr);
#endif /* !DBG */

#define EZ_DERIVE_PTK(__ad, __pmk, __a_nonce, __a_addr, __s_nonce, __s_addr, __ptk, __len) \
	WpaDerivePTK(__pmk, __a_nonce, __a_addr, __s_nonce, __s_addr, __ptk, __len);

#define EZ_GET_ENTRY_PTK(__entry) \
	&__entry->SecConfig.PTK[0]

#endif

#ifdef NEW_CONNECTION_ALGO

#define EZ_MAX_DEVICE_SUPPORT 7
#define EZ_INFO_PASS_DELAY_MSEC  1000
#define OPEN_GROUP_MAX_LEN		20
typedef struct GNU_PACKED _ez_node_number {
	UCHAR path_len; //path len is the length of the entire node number including the root_mac
	UCHAR root_mac[MAC_ADDR_LEN];
	UCHAR path[EZ_MAX_DEVICE_SUPPORT];
}EZ_NODE_NUMBER;

#ifdef EZ_ROAM_SUPPORT
struct _ez_roam_info
{
	unsigned char ez_apcli_roam_bssid[MAC_ADDR_LEN];
	unsigned char roam_channel;
	unsigned long timestamp;
};
#endif

#endif


#ifdef EZ_NETWORK_MERGE_SUPPORT
typedef enum inform_other_band_action_e
{
	ACTION_UPDATE_DUPLICATE_LINK_ENTRY,
	ACTION_UPDATE_DEVICE_INFO,
	ACTION_UPDATE_CONFIG_STATUS,
	ACTION_UPDATE_INTERNET_STATUS
	
}inform_other_band_action_t;
typedef struct GNU_PACKED weight_defining_link_s {
	void *wdev;
	ULONG time_stamp;
	ULONG ap_time_stamp;
	UCHAR peer_mac[MAC_ADDR_LEN];
	UCHAR peer_ap_mac[MAC_ADDR_LEN];
}weight_defining_link_t;

//! Levarage from MP1.0 CL 170210
#ifdef CONFIG_PUSH_VER_SUPPORT
#define NETWORK_WEIGHT_LEN  (MAC_ADDR_LEN + 2)
#else
#define NETWORK_WEIGHT_LEN  (MAC_ADDR_LEN + 1)
#endif
typedef struct GNU_PACKED channel_info_s{
	unsigned char channel;
#ifdef EZ_PUSH_BW_SUPPORT
	unsigned char ht_bw;
	unsigned char vht_bw;
#else
	unsigned char rsvd1;
	unsigned char rsvd2;
#endif
	unsigned char extcha;
}channel_info_t;
typedef struct GNU_PACKED beacon_info_tag_s
{
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	unsigned char other_ap_mac[MAC_ADDR_LEN];
	unsigned char other_ap_channel;
} beacon_info_tag_t;

typedef struct GNU_PACKED interface_info_tag_s
{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;
	channel_info_t channel_info;
	unsigned char ap_mac_addr[MAC_ADDR_LEN];//! this band AP MAC
	unsigned char cli_mac_addr[MAC_ADDR_LEN];//! this band CLI MAC
	unsigned char link_duplicate;//! when seen in CLI context it meens that other band CLI is also connected to same repeater, if seen in AP context(ez_peer) it means that both CLIs of other repeater are connected to me
#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif
} interface_info_tag_t;
typedef struct GNU_PACKED interface_info_s{
	interface_info_tag_t shared_info;
	unsigned char cli_peer_ap_mac[MAC_ADDR_LEN];//! mac address of AP to which my cli connects, will be mostly used in CLI wdev context
	BOOLEAN non_easy_connection;
	unsigned char interface_activated;
	unsigned char pmk[PMK_LEN];
}interface_info_t;

typedef struct GNU_PACKED device_info_s{

	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	weight_defining_link_t weight_defining_link;
	EZ_NODE_NUMBER ez_node_number;
} device_info_t;
#endif

#ifdef EZ_DUAL_BAND_SUPPORT
typedef enum _enum_loop_chk_role{
    NONE,
	SOURCE,
	DEST
}enum_loop_chk_role;

#define BEST_AP_RSSI_THRESHOLD_LEVEL_MAX 			ez_adapter.best_ap_rssi_threshld_max
 

typedef enum _enum_ez_api_mode
{
    FULL_OFFLOAD,
	BEST_AP_OFFLOAD,
	CONNECTION_OFFLOAD
}enum_ez_api_mode;

typedef struct GNU_PACKED _loop_chk_info {
	enum_loop_chk_role loop_chk_role;
	UCHAR source_mac[MAC_ADDR_LEN];
}LOOP_CHK_INFO;

#endif

typedef struct GNU_PACKED ez_custom_data_cmd_s {
	UINT8 data_len;
	UINT8 data_body[0];
}ez_custom_data_cmd_t, *p_ez_custom_data_cmd_t;

typedef struct GNU_PACKED ez_custom_evt_s {
	UINT8 data_len;
	UINT8 data_body[0];
}ez_custom_evt_t, *p_ez_custom_evt_t;

struct _ez_security {
	void *ad;
	void *wdev;
	unsigned char ez_band_idx;
	RALINK_TIMER_STRUCT ez_scan_timer;
	RALINK_TIMER_STRUCT ez_stop_scan_timer;
	RALINK_TIMER_STRUCT ez_scan_pause_timer;
#ifdef EZ_NETWORK_MERGE_SUPPORT
	RALINK_TIMER_STRUCT ez_group_merge_timer;
#endif
#ifdef EZ_API_SUPPORT
	enum_ez_api_mode ez_api_mode;
#endif
	NDIS_SPIN_LOCK		ez_apcli_list_sem_lock;
	LIST_HEADER			ez_apcli_list;
	unsigned char ez_scan_timer_running;
	unsigned char ez_stop_scan_timer_running;
	unsigned char ez_scan_pause_timer_running;
	NDIS_SPIN_LOCK	ez_scan_pause_timer_lock;
#ifdef EZ_NETWORK_MERGE_SUPPORT	
	unsigned char ez_group_merge_timer_running;
#endif	
	signed char best_ap_rssi_threshold;
	unsigned int capability;
	unsigned int group_id_len;
	unsigned int ez_group_id_len;	//for localy maintain EzGroupID
	unsigned int gen_group_id_len;  //for localy maintain EzGenGroupID
	unsigned char *group_id;
	unsigned char *ez_group_id;		//for localy maintain EzGroupID
	unsigned char *gen_group_id;	//for localy maintain EzGenGroupID	
	unsigned char self_dh_random_seed[EZ_RAW_KEY_LEN]; /* do NOT change after configured */
	unsigned char self_pke[EZ_RAW_KEY_LEN];
	unsigned char self_pkr[EZ_RAW_KEY_LEN];
	unsigned char self_nonce[EZ_NONCE_LEN];
	unsigned char keep_finding_provider;
	unsigned char first_scan;
	unsigned char scan_one_channel;
	unsigned char client_count;
	unsigned char go_internet;
	unsigned char user_configured;
	signed char rssi_threshold;
#ifdef EZ_NETWORK_MERGE_SUPPORT

	unsigned char merge_peer_addr[MAC_ADDR_LEN];
#ifdef DISCONNECT_ON_CONFIG_UPDATE
	unsigned char force_connect_bssid[MAC_ADDR_LEN];
	unsigned long force_bssid_timestamp;
#endif
	unsigned char weight_update_going_on; //! flag will be set when force weight is pushed, normal action frame will not be processed in this case
	unsigned char do_not_restart_interfaces;//! flag will be set while calling rtmp_set_channel to avoid CLI down/up
	unsigned char delay_disconnect_count;//! to increase beacon miss duration
    interface_info_t this_band_info;//! ssid, pmk, mac address, and CLI peer MAC address, information of wdev to which wdev correspond
	interface_info_t other_band_info_backup;//! ssid, pmk, mac address, and CLI peer MAC address, information of wdev to which wdev correspond
#if (defined(DOT11_N_SUPPORT) && defined(DOT11N_DRAFT3))
	BOOLEAN ap_did_fallback;
	unsigned char fallback_channel;
#endif

#endif
#ifdef NEW_CONNECTION_ALGO
	
	BOOLEAN ez_apcli_immediate_connect;
	BOOLEAN ez_connection_permission_backup;
	BOOLEAN ez_is_connection_allowed;
	unsigned char ez_apcli_force_ssid[MAX_LEN_OF_SSID];
	unsigned char ez_apcli_force_ssid_len;
	unsigned char ez_apcli_force_bssid[MAC_ADDR_LEN];
	unsigned char ez_apcli_force_channel;
	
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];
#ifdef EZ_ROAM_SUPPORT
	struct _ez_roam_info ez_roam_info;
	UCHAR ez_ap_roam_blocked_mac[MAC_ADDR_LEN];
#endif

#ifdef EZ_DUAL_BAND_SUPPORT
	BOOLEAN internal_force_connect_bssid;
	BOOLEAN internal_force_connect_bssid_timeout;
	unsigned long force_connect_bssid_time_stamp;
    LOOP_CHK_INFO loop_chk_info;
	RALINK_TIMER_STRUCT ez_loop_chk_timer;
	unsigned char ez_loop_chk_timer_running;
	BOOLEAN first_loop_check;
	BOOLEAN dest_loop_detect;
#endif
	unsigned char ez_action_type;
	UINT32 ez_scan_delay;
	UINT32 ez_max_scan_delay;
	BOOLEAN ez_scan;
	BOOLEAN ez_scan_same_channel;
	ULONG	ez_scan_same_channel_timestamp;
#endif
	BOOLEAN ez_wps_reconnect;
	unsigned char ez_wps_bssid[6];
	ULONG	ez_wps_reconnect_timestamp;
	BOOLEAN disconnect_by_ssid_update;

	unsigned char default_ssid[MAX_LEN_OF_SSID];
	unsigned char default_ssid_len;
	unsigned char default_pmk[PMK_LEN];
	unsigned char default_pmk_valid;
};


typedef struct __ez_triband_sec_config
{
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;    
	UINT32 AKMMap;
	
} EZ_TRIBAND_SEC_CONFIG;


typedef struct __non_ez_band_info_tag{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	EZ_TRIBAND_SEC_CONFIG triband_sec;	
	unsigned char encrypted_pmk[PMK_LEN + EZ_AES_KEY_ENCRYPTION_EXTEND];
	//unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif

}NON_EZ_BAND_INFO_TAG;

//! Leverage form MP.1.0 CL 170037
typedef struct __non_man_info_tag{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
//! Leverage form MP.1.0 CL 170364
	unsigned char encryptype[32];
	unsigned char authmode[32];

#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif

} NON_MAN_INFO_TAG;

typedef struct __non_ez_band_psk_info_tag{
	unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
}NON_EZ_BAND_PSK_INFO_TAG;

typedef struct __non_ez_band_info{
	void *  pAd;
	CHAR func_idx; 
	struct wifi_dev *non_ez_ap_wdev;
	struct wifi_dev *non_ez_cli_wdev;
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	EZ_TRIBAND_SEC_CONFIG triband_sec;	
	BOOLEAN need_restart;
	unsigned char pmk[PMK_LEN];
	
	unsigned char psk[LEN_PSK];
#ifdef DOT11R_FT_SUPPORT
		UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
		UINT8 rsvd[2];
#endif
	
} NON_EZ_BAND_INFO;

//! Levarage from MP.1.0 CL #170037
typedef struct __non_man_info{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	unsigned char psk[LEN_PSK];
//! Leverage form MP.1.0 CL 170364
	unsigned char encryptype[32];
	unsigned char authmode[32];

#ifdef DOT11R_FT_SUPPORT
		UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
		UINT8 rsvd[2];
#endif
	
} NON_MAN_INFO;


typedef struct GNU_PACKED updated_configs_s{
	unsigned char mac_addr[MAC_ADDR_LEN];//! mac addr of peer from which we received configs
	BOOLEAN context_linkdown;
	unsigned char *group_id;
	unsigned int group_id_len;
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];
	device_info_t device_info;//! updated device info
	interface_info_t this_band_info;//! updated
	interface_info_t other_band_info;
	NON_EZ_BAND_INFO_TAG non_ez_info[2];
	NON_EZ_BAND_PSK_INFO_TAG non_ez_psk_info[2];
	BOOLEAN need_ez_update;
	BOOLEAN need_non_ez_update_ssid[2];
	BOOLEAN need_non_ez_update_psk[2];
	BOOLEAN need_non_ez_update_secconfig[2];
	//! Levarage from MP1.0 CL #170037
	NON_MAN_INFO_TAG non_man_info;

}updated_configs_t;
struct _ez_peer_security_info {
	struct wifi_dev *wdev;
	unsigned int capability;
	unsigned int group_id_len;
	unsigned int gtk_len;
	unsigned char *group_id;
	unsigned char *gtk;
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char peer_pke[EZ_RAW_KEY_LEN];
	unsigned char peer_nonce[EZ_NONCE_LEN];
	unsigned char dh_key[EZ_DH_KEY_LEN];
	unsigned char sw_key[EZ_PTK_LEN];
	//unsigned char pmk[EZ_PMK_LEN];
	unsigned char valid;

#if defined (NEW_CONNECTION_ALGO) || defined (EZ_NETWORK_MERGE_SUPPORT)
	unsigned char port_secured;
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];

#endif

#ifdef EZ_NETWORK_MERGE_SUPPORT	
#ifdef EZ_DUAL_BAND_SUPPORT
	device_info_t device_info;// when device info is used in ez_peer, only node number is expected to give correct information, others are session variable which does not hold any significance after comparision
	interface_info_t this_band_info;
	interface_info_t other_band_info;
#else
	unsigned char ssid_len;
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char other_band_ssid_len;
	unsigned char other_band_ssid[MAX_LEN_OF_SSID];
	unsigned char other_band_pmk[PMK_LEN];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	unsigned char ap_mac_addr[MAC_ADDR_LEN];
	unsigned char target_channel;
	unsigned char ht_bw;
	unsigned char vht_bw;
	unsigned char ext_cha_offset;
#endif	
#endif	

#ifdef NEW_CONNECTION_ALGO
	unsigned long creation_time;
	unsigned char ez_peer_table_index;
#endif
	BOOLEAN delete_in_differred_context;
	BOOLEAN ez_disconnect_due_roam;
	NON_EZ_BAND_INFO_TAG non_ez_band_info[MAX_NON_EZ_BANDS];
	NON_EZ_BAND_PSK_INFO_TAG non_ez_psk_info[MAX_NON_EZ_BANDS];
	//! Levarage from MP1.0 CL#170037
	NON_MAN_INFO_TAG non_man_info;
};

struct _ez_apcli_entry {
	struct _ez_apcli_entry *next;
	unsigned long receive_time;
	unsigned char mac_addr[MAC_ADDR_LEN];
};
struct ez_GUI_info 
{
    unsigned char EzEnable;
    unsigned char EzConfStatus;
    unsigned char EzGroupID[248];
    unsigned char EzGenGroupId[32];
	unsigned char EzOpenGroupID[20];
    unsigned char ApCliEzConfStatus;
    unsigned char ApCliEzGroupID[248];
	unsigned char ApCliEzGenGroupId[32];
	unsigned char ApCliEzOpenGroupID[20];
	unsigned char ApCliHideSSID[32];
    unsigned char ApCliAuthMode[13];
    unsigned char ApCliEncrypType[7];
    unsigned char ApCliWPAPSK[64];
	unsigned char ApCliSsid[32];
    BOOLEAN ApCliEnable;
    unsigned char ApCliEzEnable;
};

typedef struct __ez_adapter{
	unsigned char band_count;
	EZ_BAND_INFO ez_band_info[MAX_EZ_BANDS];
	unsigned char non_ez_band_count;
	NON_EZ_BAND_INFO non_ez_band_info[MAX_NON_EZ_BANDS];
	unsigned int backhaul_channel;
	unsigned int front_end_channel;
	
	RTMP_OS_TASK restartNonEzApTask;
	
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */
	//RTMP_NET_TASK_STRUCT restartNonEzApTasklet;
#ifdef EZ_PUSH_BW_SUPPORT
	BOOLEAN push_bw_config;
#endif
	UINT8 ez_roam_time;
	unsigned char ez_delay_disconnect_count;
	UINT8 ez_wait_for_info_transfer;
	UINT8 ez_wdl_missing_time;
	UINT32 ez_force_connect_bssid_time;
	UINT8 ez_peer_entry_age_out_time;
	UINT8 ez_scan_same_channel_time;
	UINT32 ez_partial_scan_time;
	signed char best_ap_rssi_threshld[10];
	unsigned char best_ap_rssi_threshld_max;

	RALINK_TIMER_STRUCT ez_connect_wait_timer;
	unsigned char ez_connect_wait_timer_running;
	unsigned long ez_connect_wait_timer_value;
	unsigned long ez_connect_wait_timer_timestamp;
	unsigned char configured_status; /* 0x01 - un-configured, 0x02 - configured */
	device_info_t device_info;//! network weight, node number and weight defining link info, all interface should have same content in htis structure
	char Peer2p4mac[MAC_ADDR_LEN];
	//! Levarage from MP1.0 CL#170037
	unsigned char is_man_nonman;
	NON_MAN_INFO non_man_info;	
//! Levarage from MP1.0 CL 170210
//! repeater device flag removed from ez_adapter
} EZ_ADAPTER;

typedef struct device_info_to_app_s
{
	unsigned char dual_chip_dbdc;
	unsigned char ssid_len1;
	unsigned char ssid_len2;
	unsigned char internet_access;
	char ssid1[MAX_LEN_OF_SSID];
	char ssid2[MAX_LEN_OF_SSID];
	unsigned char pmk1[LEN_PMK];
	unsigned char pmk2[LEN_PMK];
	unsigned char device_connected[2];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer2p4mac[MAC_ADDR_LEN];
	unsigned char non_ez_connection;
	unsigned char update_parameters;
	unsigned char is_forced;
	unsigned char third_party_present;
	unsigned char new_updated_received;
	unsigned char is_push;	
	unsigned char sta_cnt;
	unsigned char stamac[10][MAC_ADDR_LEN];	
	
} device_info_to_app_t;

typedef struct triband_ez_device_info_to_app_s
{
	unsigned char ssid_len;
	unsigned char internet_access;
	unsigned char is_non_ez_connection;
	char ssid[MAX_LEN_OF_SSID];
	char non_ez_ssid1[MAX_LEN_OF_SSID];
	char non_ez_ssid2[MAX_LEN_OF_SSID];
	unsigned char non_ez_ssid1_len;
	unsigned char non_ez_ssid2_len;
	unsigned char need_non_ez_update_ssid[2];
	unsigned char pmk[LEN_PMK];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer_mac[MAC_ADDR_LEN];
	unsigned char is_forced;	
	unsigned char update_parameters;
	unsigned char third_party_present;
	unsigned char new_updated_received;
} triband_ez_device_info_to_app_t;


typedef struct triband_non_ez_device_info_to_app_s
{
	unsigned char non_ez_psk1[LEN_PSK];
	unsigned char non_ez_psk2[LEN_PSK];
	unsigned char non_ez_auth_mode1[20];
	unsigned char non_ez_auth_mode2[20];	
	unsigned char non_ez_encryptype1[20];
	unsigned char non_ez_encryptype2[20];		
	unsigned char need_non_ez_update_psk[2];
	unsigned char need_non_ez_update_secconfig[2];
} triband_nonez_device_info_to_app_t;
//! Levarage from MP1.0 CL #170037
typedef struct man_plus_nonman_ez_device_info_to_app_s
{
	unsigned char ssid_len;
	unsigned char internet_access;
	char is_non_ez_connection;
	char ssid[MAX_LEN_OF_SSID];
	unsigned char pmk[LEN_PMK];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer_mac[MAC_ADDR_LEN];	
	char update_parameters;
	char third_party_present;
	char new_updated_received;
} man_plus_nonman_ez_device_info_to_app_t;


typedef struct man_plus_nonman_non_ez_device_info_to_app_s
{
	unsigned char non_ez_ssid[MAX_LEN_OF_SSID];
//! Leverage form MP.1.0 CL 170364
	unsigned char non_ez_ssid_len;
	unsigned char non_ez_psk[LEN_PSK];
//! Leverage form MP.1.0 CL 170364
	unsigned char non_ez_encryptype[32];
	unsigned char non_ez_auth_mode[32];
#ifdef DOT11R_FT_SUPPORT	
	UINT8 ftmdid[FT_MDID_LEN];
#endif	
//! Leverage form MP.1.0 CL 170364
	unsigned char need_non_ez_update_ssid;
	unsigned char need_non_ez_update_psk;
	unsigned char need_non_ez_update_secconfig;
} man_plus_nonman_nonez_device_info_to_app_t;


typedef struct web_conf_info_s{
	unsigned char data_len;
	char data[250];	
} web_conf_info_t;

extern EZ_ADAPTER ez_adapter;
void ez_start(
	void *wdev_obj,
	unsigned char ap_mode);
void ez_stop(
	void *wdev_obj);
void ez_init(
	void *ad_obj,
	void *wdev_obj,
	unsigned char ap_mode);
void ez_exit(
	void *wdev_obj);
void ez_peer_table_delete(
	void *wdev_obj,
	unsigned char *addr);
struct _ez_peer_security_info *ez_peer_table_search_by_addr(
	void *wdev_obj,
	unsigned char *addr);
unsigned long ez_build_beacon_ie(
	void *wdev_obj,
	unsigned char *frame_buf);
unsigned long ez_build_probe_request_ie(
	void *wdev_obj,
	unsigned char *frame_buf);
unsigned long ez_build_probe_response_ie(
	void *wdev_obj,
	unsigned char *frame_buf);
unsigned long ez_build_auth_request_ie(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *frame_buf);
unsigned long ez_build_assoc_request_ie(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *frame_buf,
	unsigned int frame_len);
unsigned long ez_build_assoc_response_ie(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *ap_gtk,
	unsigned int ap_gtk_len,
	unsigned char *frame_buf);
void ez_process_beacon_probe_response(
	struct _ez_security *ez_sec_info,
	void *msg,
	unsigned long msg_len);
unsigned char ez_process_probe_request(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len);
unsigned char ez_process_auth_request(
	void *ad_obj,
	void *wdev_obj,
	void *auth_info_obj,
	void *msg,
	unsigned long msg_len);
void ez_process_auth_response(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned long *current_state,
	void *msg,
	unsigned long msg_len);
unsigned short ez_process_assoc_request(
	void *wdev_obj,
	void *entry_obj,
	unsigned char isReassoc,
	void *msg,
	unsigned long msg_len);
unsigned short ez_process_assoc_response(
	void *wdev_obj,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len);
unsigned char ez_install_ptk(
	void *ad_obj,
	void *entry_obj,
	unsigned char authenticator);
#ifdef APCLI_SUPPORT
unsigned char ez_apcli_install_gtk(
	void *ad_obj,
	void *entry_obj);
void ez_apcli_link_down(
	void *ad_obj,
	void *apcli_obj,
	unsigned char if_idx);
void ez_apclient_info_action_frame(
	void *ad_obj,
	void *wdev_obj);
#endif /* APCLI_SUPPORT */
void ez_process_action_frame(
	void *ad_obj,
	void *elem_obj);
void ez_prepare_security_key(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char authenticator);
unsigned char ez_search_provider(
	void *ad_obj,
	void *apcli_entry_obj,
	unsigned char inf_idx);
unsigned char ez_avoid_looping_connection(
	void *ad_obj,
	void *apcli_entry_obj,
	unsigned char if_idx);
void increment_best_ap_rssi_threshold(struct _ez_security *ez_security);

void ez_show_information(
	void *wdev_obj);
VOID ez_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
VOID ez_stop_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
VOID ez_scan_pause_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#ifdef EZ_NETWORK_MERGE_SUPPORT	
VOID ez_group_merge_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif	
BOOLEAN ez_port_secured(
	void *ad_obj,
	void *entry_obj,
	unsigned char if_idx,
	unsigned char ap_mode);
void ez_read_parms_from_file(
	void *ad_obj,
	char *tmpbuf,
	char *buffer);
unsigned char ez_set_group_id(
	struct _ez_security *ez_sec_info,
	unsigned char *group_id,
	unsigned int group_id_len,
	unsigned char inf_idx);
unsigned char ez_set_ezgroup_id(
	struct _ez_security *ez_sec_info,
	unsigned char *group_id,
	unsigned int group_id_len,
	unsigned char inf_idx);
unsigned char ez_set_gen_group_id(
	struct _ez_security *ez_sec_info,
	unsigned char *gen_group_id,
	unsigned int gen_group_id_len,
	unsigned char inf_idx);
void ez_del_apcli_entry_by_mac(
	LIST_HEADER *ez_apcli_list,
	unsigned char *peer_addr);
void ez_clear_apcli_list(
	LIST_HEADER	*apcli_list);
int ez_Internet_MsgHandle(
	void *ad_obj, 
	RTMP_IOCTL_INPUT_STRUCT *wrq);
INT Custom_DataHandle(
	void *ad_obj, 
	RTMP_IOCTL_INPUT_STRUCT *wrq);
INT Custom_EventHandle(void *ad_obj, 
	ez_custom_data_cmd_t *data, 
	unsigned char datalen);
BOOLEAN send_action_custom_data(
	void *ad_obj, 
	struct wifi_dev *wdev, 
	struct _ez_peer_security_info *ez_peer, 
	ez_custom_data_cmd_t *data, 
	unsigned char datalen);

BOOLEAN ez_ApCliAutoConnectBWAdjust(IN void	*ad_obj,IN struct wifi_dev	*wdev,IN void	*bss_entry_obj);	
#ifdef NEW_CONNECTION_ALGO

unsigned char ez_set_open_group_id(
	struct _ez_security *ez_sec_info,
	unsigned char *open_group_id,
	unsigned int open_group_id_len,
	unsigned char inf_idx);

void ez_update_capability(
	void *ad_obj, 
	void *wdev_obj);	
#endif
void ez_send_unicast_deauth_apcli(void *ad_obj, USHORT idx);
void ez_update_connection(void *ad_obj,
	void *wdev_obj);
BOOLEAN ez_get_other_band_info(void * ad_obj, void * wdev_obj, void *other_band_config);

#ifdef EZ_NETWORK_MERGE_SUPPORT	


#define GET_MAX_UCAST_NUM(_pAd) HcGetMaxStaNum(_pAd)
#define BROADCAST_WCID          0xFF
typedef enum enum_config_update_action
{
	ACTION_NOTHING,
	ACTION_PUSH,
	ACTION_ADAPT	
		
}enum_config_update_action_t;
typedef enum enum_group_merge_action
{
	EXIT_SWITCH_NOT_GROUP_MERGE,
	TERMINATE_LOOP_MULTIPLE_AP_FOUND,
	TERMINATE_LOOP_TARGET_AP_FOUND,
	CONTINUE_LOOP_TARGET_AP_FOUND,
	CONTINUE_LOOP
}enum_group_merge_action_t;

void ez_process_action_frame(
	void *ad_obj,
	void *elem_obj);
void send_action_delay_disconnect(void *ad_obj, struct wifi_dev *wdev, struct _ez_peer_security_info *ez_peer, unsigned char delay_disconnect);

BOOLEAN ez_is_weight_same(
	PUCHAR own_weight, 
	PUCHAR peer_weight);
void update_and_push_weight(void *ad_obj, void * entry_obj, unsigned char * network_weight);
void send_action_update_weight(void *ad_obj,
					unsigned char *mac_addr,
					struct wifi_dev *wdev, 
					unsigned char * network_weight);

#endif


#ifdef NEW_CONNECTION_ALGO

#define EZ_WAIT_FOR_INFO_TRANSFER 1000 // 1 sec
#define EZ_WAIT_FOR_ROAM_COMPLETE 60000 // 60 sec
#define EZ_MAX_SCAN_DELAY 30000
#define EZ_SCAN_DELAY_WAIT 10000
#define EZ_SEC_TO_MSEC 1000 // 1 sec

#define EZ_DELAY_DISCONNECT_FOR_PBC 4
enum EZ_CONN_ACTION
{
	EZ_ALLOW_ALL,
	EZ_DISALLOW_ALL,
	EZ_ADD_DISALLOW,
	EZ_ADD_ALLOW,
	EZ_DISALLOW_ALL_ALLOW_ME,
	EZ_ALLOW_ALL_TIMEOUT,
	EZ_ENQUEUE_PERMISSION,
	EZ_DEQUEUE_PERMISSION,
};




void ez_allocate_node_number_sta(
	void *ad_obj,
	unsigned char if_index,
	void * entry_obj,
	BOOLEAN is_forced);

/*check whether peer node is child node of the own node*/
BOOLEAN ez_is_child_node(
	EZ_NODE_NUMBER own_node_number, 
	EZ_NODE_NUMBER peer_node_number);



BOOLEAN ez_apcli_search_best_ap(
	void *ad_obj,
	void *apcli_entry_obj,
	unsigned char inf_idx);

void ez_apcli_force_ssid(
	void *ad_obj,
	UCHAR if_index, 
	unsigned char *ssid, 
	unsigned char ssid_len);

void ez_apcli_force_bssid(
	void *ad_obj,
	unsigned char if_index, 
	unsigned char *bssid);

void ez_apcli_force_channel(
	void *ad_obj,
	unsigned char if_index, 
	unsigned char channel);



BOOLEAN ez_apcli_transfer_node_number(
	void *ad_obj,
	unsigned char if_index);

BOOLEAN ez_is_connection_allowed(
	struct wifi_dev *wdev);

BOOLEAN ez_update_connection_permission(
	void *ad_obj,
	struct wifi_dev *wdev, 
	enum EZ_CONN_ACTION action);


void ez_wait_for_connection_allow_timeout(
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3);

void ez_wait_for_connection_allow(
	unsigned long time);

void ez_peer_table_maintenance(
	void *ad_obj);

#endif

void ez_handle_peer_disconnection(struct wifi_dev *wdev, unsigned char * mac_addr);
void ez_allocate_or_update_non_ez_band(void *wdev_obj);

void ez_restore_channel_config(struct wifi_dev *wdev);
#if (defined(DOT11_N_SUPPORT) && defined(DOT11N_DRAFT3))
void ez_set_ap_fallback_context(struct wifi_dev *wdev, BOOLEAN fallback, unsigned char fallback_channel);
#endif

void ez_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

#ifdef EZ_DUAL_BAND_SUPPORT

/* Indicates whether grp Tx pkt needs to be dropped by apcli*/
BOOLEAN ez_apcli_tx_grp_pkt_drop(
    IN struct wifi_dev *wdev,
    IN struct sk_buff *pSkb);

void ez_apcli_uni_tx_on_dup_link(
	IN struct wifi_dev *wdev,
	IN struct sk_buff *pSkb);

/* Indicates whether grp Tx pkt needs to be dropped by Ap for duplicate CLI links*/
BOOLEAN ez_ap_tx_grp_pkt_drop_to_ez_apcli(
	IN struct wifi_dev *wdev,
	IN struct sk_buff *pSkb);

BOOLEAN ez_apcli_rx_grp_pkt_drop(
	IN struct wifi_dev *wdev,
	IN struct sk_buff *pSkb);

/* Loop Check timeout handler*/
void ez_loop_chk_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

/* Trigger Loop check process when both CLI connected to non-easy root APs*/
void ez_chk_loop_thru_non_ez_ap(
	IN void *ad_obj,
	IN void *pMacEntry);

/* Process Rx Pkt for loop pkt detection*/
BOOLEAN ez_is_loop_pkt_rcvd(
	IN void * wdev_obj,
	IN void *pRxBlk);

#endif
void ez_handle_action_txstatus(void *ad_obj, unsigned int wcid);
void ez_send_unicast_deauth(void *ad_obj, UCHAR *peer_addr);
void ez_apcli_check_partial_scan(void *ad_obj, CHAR apcli_idx);
struct wifi_dev * ez_get_otherband_wdev(void *wdev_obj);
struct wifi_dev * ez_get_otherband_ap_wdev(void *wdev_obj);
struct wifi_dev * ez_get_otherband_cli_wdev(void *wdev_obj);
void * ez_get_otherband_ad(void *wdev_obj);
void ez_dealloc_band(void *wdev_obj);
void ez_allocate_or_update_band(void *wdev_obj);
unsigned short ez_check_for_ez_enable(
		struct wifi_dev *wdev,
		void *msg,
		unsigned long msg_len
		);

void ez_insert_tlv(
	unsigned char tag,
	unsigned char *data,
	unsigned char data_len,
	unsigned char *buffer,
	unsigned long *msg_len);

unsigned long ez_build_non_ez_beacon_ie(
	void *wdev_obj,
	unsigned char *frame_buf);

BOOLEAN ez_is_triband(void );
void ez_dealloc_non_ez_band(void *wdev_obj);


#endif /* WH_EZ_SETUP */

#endif /* __EZ_CMM_H__ */
