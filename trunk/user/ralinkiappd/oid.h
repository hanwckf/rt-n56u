#ifndef _OID_H
#define _OID_H


#include <linux/if_ether.h>
#include <linux/wireless.h>

#define RT_PRIV_IOCTL                           (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET                        (SIOCIWFIRSTPRIV + 0x02)

#ifdef DBG
#define RTPRIV_IOCTL_BBP			(SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC			(SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P			(SIOCIWFIRSTPRIV + 0x07)
#endif

#define RTPRIV_IOCTL_STATISTICS			(SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE		(SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA		(SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY		(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY		(SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_STATIC_WEP_COPY		(SIOCIWFIRSTPRIV + 0x10)
#define RTPRIV_IOCTL_WSC_PROFILE		(SIOCIWFIRSTPRIV + 0x12)

#define OID_GET_SET_TOGGLE                          0x8000
#define OID_802_11_NETWORK_TYPES_SUPPORTED          0x0103
#define OID_802_11_NETWORK_TYPE_IN_USE              0x0104
#define OID_802_11_RSSI_TRIGGER                     0x0107
#define RT_OID_802_11_RSSI                          0x0108
#define RT_OID_802_11_RSSI_1                        0x0109
#define RT_OID_802_11_RSSI_2                        0x010A
#define OID_802_11_NUMBER_OF_ANTENNAS               0x010B
#define OID_802_11_RX_ANTENNA_SELECTED              0x010C
#define OID_802_11_TX_ANTENNA_SELECTED              0x010D
#define OID_802_11_SUPPORTED_RATES                  0x010E
#define OID_802_11_ADD_WEP                          0x0112
#define OID_802_11_REMOVE_WEP                       0x0113
#define OID_802_11_DISASSOCIATE                     0x0114
#define OID_802_11_PRIVACY_FILTER                   0x0118
#define OID_802_11_ASSOCIATION_INFORMATION          0x011E
#define OID_802_11_TEST                             0x011F
#define RT_OID_802_11_COUNTRY_REGION                0x0507
#define OID_802_11_BSSID_LIST_SCAN                  0x0508
#define OID_802_11_SSID                             0x0509
#define OID_802_11_BSSID                            0x050A
#define RT_OID_802_11_RADIO                         0x050B
#define RT_OID_802_11_PHY_MODE                      0x050C
#define RT_OID_802_11_STA_CONFIG                    0x050D
#define OID_802_11_DESIRED_RATES                    0x050E
#define RT_OID_802_11_PREAMBLE                      0x050F
#define OID_802_11_WEP_STATUS                       0x0510
#define OID_802_11_AUTHENTICATION_MODE              0x0511
#define OID_802_11_INFRASTRUCTURE_MODE              0x0512
#define RT_OID_802_11_RESET_COUNTERS                0x0513
#define OID_802_11_RTS_THRESHOLD                    0x0514
#define OID_802_11_FRAGMENTATION_THRESHOLD          0x0515
#define OID_802_11_POWER_MODE                       0x0516
#define OID_802_11_TX_POWER_LEVEL                   0x0517
#define RT_OID_802_11_ADD_WPA                       0x0518
#define OID_802_11_REMOVE_KEY                       0x0519
#define OID_802_11_ADD_KEY                          0x0520
#define OID_802_11_CONFIGURATION                    0x0521
#define OID_802_11_TX_PACKET_BURST                  0x0522
#define RT_OID_802_11_QUERY_NOISE_LEVEL             0x0523
#define RT_OID_802_11_EXTRA_INFO                    0x0524

#define OID_802_11_ENCRYPTION_STATUS                OID_802_11_WEP_STATUS

#define RT_OID_DEVICE_NAME                          0x0607
#define RT_OID_VERSION_INFO                         0x0608
#define OID_802_11_BSSID_LIST                       0x0609
#define OID_802_3_CURRENT_ADDRESS                   0x060A
#define OID_GEN_MEDIA_CONNECT_STATUS                0x060B
#define RT_OID_802_11_QUERY_LINK_STATUS             0x060C
#define OID_802_11_RSSI                             0x060D
#define OID_802_11_STATISTICS                       0x060E
#define OID_GEN_RCV_OK                              0x060F
#define OID_GEN_RCV_NO_BUFFER                       0x0610
#define RT_OID_802_11_QUERY_EEPROM_VERSION          0x0611
#define RT_OID_802_11_QUERY_FIRMWARE_VERSION        0x0612
#define RT_OID_802_11_QUERY_LAST_RX_RATE            0x0613
#define RT_OID_802_11_TX_POWER_LEVEL_1              0x0614
#define RT_OID_802_11_QUERY_PIDVID                  0x0615

#define OID_SET_COUNTERMEASURES                     0x0616
#define OID_802_11_SET_IEEE8021X                    0x0617
#define OID_802_11_SET_IEEE8021X_REQUIRE_KEY        0x0618
#define OID_802_11_PMKID                            0x0620
#define RT_OID_WE_VERSION_COMPILED                  0x0622

#define	RT_OID_802_11_SNR_0                         0x0630
#define	RT_OID_802_11_SNR_1                         0x0631
#define	RT_OID_802_11_QUERY_LAST_TX_RATE            0x0632
#define	RT_OID_802_11_QUERY_HT_PHYMODE              0x0633
#define	RT_OID_802_11_SET_HT_PHYMODE                0x0634
#define	OID_802_11_RELOAD_DEFAULTS                  0x0635
#define	RT_OID_802_11_QUERY_APSD_SETTING            0x0636
#define	RT_OID_802_11_SET_APSD_SETTING              0x0637
#define	RT_OID_802_11_QUERY_APSD_PSM                0x0638
#define	RT_OID_802_11_SET_APSD_PSM                  0x0639
#define	RT_OID_802_11_QUERY_DLS                     0x063A
#define	RT_OID_802_11_SET_DLS                       0x063B
#define	RT_OID_802_11_QUERY_DLS_PARAM               0x063C
#define	RT_OID_802_11_SET_DLS_PARAM                 0x063D
#define RT_OID_802_11_QUERY_WMM                     0x063E
#define RT_OID_802_11_SET_WMM                       0x063F
#define RT_OID_802_11_QUERY_IMME_BA_CAP             0x0640
#define RT_OID_802_11_SET_IMME_BA_CAP               0x0641
#define RT_OID_802_11_QUERY_BATABLE                 0x0642
#define RT_OID_802_11_ADD_IMME_BA                   0x0643
#define RT_OID_802_11_TEAR_IMME_BA                  0x0644
#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_802_11_QUERY_DAT_HT_PHYMODE          0x0646
#define RT_OID_QUERY_MULTIPLE_CARD_SUPPORT          0x0647
#define OID_802_11_SET_PSPXLINK_MODE                0x0648
#define OID_802_11_SET_PASSPHRASE                   0x0649
#define RT_OID_802_11_SNR_2                         0x064a
#define RT_OID_802_11_STREAM_SNR                    0x064b
#define RT_OID_802_11_QUERY_TXBF_TABLE              0x0650
#define RT_OID_802_11_WSC_QUERY_PROFILE             0x0750
#define RT_OID_WSC_UUID                             0x0753

// IAPPD/802.11R oids
#define RT_QUERY_SIGNAL_CONTEXT                     0x0402
#define RT_SET_IAPP_PID                             0x0404
#define RT_SET_APD_PID                              0x0405
#define RT_SET_DEL_MAC_ENTRY                        0x0406
#define RT_QUERY_EVENT_TABLE                        0x0407
#define RT_SET_FT_STATION_NOTIFY                    0x0408
#define RT_SET_FT_KEY_REQ                           0x0409
#define RT_SET_FT_KEY_RSP                           0x040a
#define RT_FT_KEY_SET                               0x040b
#define RT_FT_DATA_ENCRYPT                          0x040c
#define RT_FT_DATA_DECRYPT                          0x040d
#define RT_FT_NEIGHBOR_REPORT                       0x040e
#define RT_FT_NEIGHBOR_REQUEST                      0x040f
#define RT_FT_NEIGHBOR_RESPONSE                     0x0410
#define RT_FT_ACTION_FORWARD                        0x0411

// Ralink defined OIDs
// Dennis Lee move to platform specific
#define RT_OID_802_11_BSSID                   (OID_GET_SET_TOGGLE | OID_802_11_BSSID)
#define RT_OID_802_11_SSID                    (OID_GET_SET_TOGGLE | OID_802_11_SSID)
#define RT_OID_802_11_INFRASTRUCTURE_MODE     (OID_GET_SET_TOGGLE | OID_802_11_INFRASTRUCTURE_MODE)
#define RT_OID_802_11_ADD_WEP                 (OID_GET_SET_TOGGLE | OID_802_11_ADD_WEP)
#define RT_OID_802_11_ADD_KEY                 (OID_GET_SET_TOGGLE | OID_802_11_ADD_KEY)
#define RT_OID_802_11_REMOVE_WEP              (OID_GET_SET_TOGGLE | OID_802_11_REMOVE_WEP)
#define RT_OID_802_11_REMOVE_KEY              (OID_GET_SET_TOGGLE | OID_802_11_REMOVE_KEY)
#define RT_OID_802_11_DISASSOCIATE            (OID_GET_SET_TOGGLE | OID_802_11_DISASSOCIATE)
#define RT_OID_802_11_AUTHENTICATION_MODE     (OID_GET_SET_TOGGLE | OID_802_11_AUTHENTICATION_MODE)
#define RT_OID_802_11_PRIVACY_FILTER          (OID_GET_SET_TOGGLE | OID_802_11_PRIVACY_FILTER)
#define RT_OID_802_11_BSSID_LIST_SCAN         (OID_GET_SET_TOGGLE | OID_802_11_BSSID_LIST_SCAN)
#define RT_OID_802_11_WEP_STATUS              (OID_GET_SET_TOGGLE | OID_802_11_WEP_STATUS)
#define RT_OID_802_11_RELOAD_DEFAULTS         (OID_GET_SET_TOGGLE | OID_802_11_RELOAD_DEFAULTS)
#define RT_OID_802_11_NETWORK_TYPE_IN_USE     (OID_GET_SET_TOGGLE | OID_802_11_NETWORK_TYPE_IN_USE)
#define RT_OID_802_11_TX_POWER_LEVEL          (OID_GET_SET_TOGGLE | OID_802_11_TX_POWER_LEVEL)
#define RT_OID_802_11_RSSI_TRIGGER            (OID_GET_SET_TOGGLE | OID_802_11_RSSI_TRIGGER)
#define RT_OID_802_11_FRAGMENTATION_THRESHOLD (OID_GET_SET_TOGGLE | OID_802_11_FRAGMENTATION_THRESHOLD)
#define RT_OID_802_11_RTS_THRESHOLD           (OID_GET_SET_TOGGLE | OID_802_11_RTS_THRESHOLD)
#define RT_OID_802_11_RX_ANTENNA_SELECTED     (OID_GET_SET_TOGGLE | OID_802_11_RX_ANTENNA_SELECTED)
#define RT_OID_802_11_TX_ANTENNA_SELECTED     (OID_GET_SET_TOGGLE | OID_802_11_TX_ANTENNA_SELECTED)
#define RT_OID_802_11_SUPPORTED_RATES         (OID_GET_SET_TOGGLE | OID_802_11_SUPPORTED_RATES)
#define RT_OID_802_11_DESIRED_RATES           (OID_GET_SET_TOGGLE | OID_802_11_DESIRED_RATES)
#define RT_OID_802_11_CONFIGURATION           (OID_GET_SET_TOGGLE | OID_802_11_CONFIGURATION)
#define RT_OID_802_11_POWER_MODE              (OID_GET_SET_TOGGLE | OID_802_11_POWER_MODE)

/* for WPS --YY  */
#define RT_OID_SYNC_RT61                      0x0D010750
#define RT_OID_WSC_QUERY_STATUS               ((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE                   (RT_OID_SYNC_RT61 + 0x02) & 0xffff)

#define NDIS_802_11_LENGTH_SSID			32
#define NDIS_802_11_LENGTH_RATES		8
#define NDIS_802_11_LENGTH_RATES_EX		16

#define NdisMediaStateConnected                 1
#define NdisMediaStateDisconnected              0

// PhyMODE
#define MODE_CCK		0
#define MODE_OFDM   		1
#define MODE_HTMIX		2
#define MODE_HTGREENFIELD	3
#define MODE_VHT		4

// BW
#define BW_20                   0
#define BW_40                   1
#define BW_80                   2
#define BW_160                  3
#define BW_10                   4
#define BW_5                    5
#define BW_8080	                6

// SHORTGI
#define GI_400      1 // only support in HT mode
#define GI_800      0

#define WPA_OUI_TYPE        0x01F25000
#define WPA_OUI             0x00F25000
#define WPA_OUI_1           0x00030000
#define WPA2_OUI            0x00AC0F00
#define CISCO_OUI           0x00964000

#define MAX_NUMBER_OF_MAC       56
#define MAX_NUMBER_OF_BSSID     4

typedef unsigned long   NDIS_802_11_FRAGMENTATION_THRESHOLD;
typedef unsigned long   NDIS_802_11_RTS_THRESHOLD;
typedef unsigned long   NDIS_802_11_TX_POWER_LEVEL; // in milliwatts
typedef unsigned long long NDIS_802_11_KEY_RSC;

#define PACKED __attribute__ ((packed))

#define A_SHA_DIGEST_LEN 20
typedef struct
{
	unsigned int    H[5];
	unsigned int    W[80];
	int             lenW;
	unsigned int    sizeHi,sizeLo;
} A_SHA_CTX;

typedef struct PACKED _NDIS_802_11_SSID
{
	unsigned int    SsidLength;   // length of SSID field below, in bytes;
                                  // this can be zero.
	unsigned char   Ssid[NDIS_802_11_LENGTH_SSID]; // SSID information field
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

typedef struct _NDIS_802_11_CONFIGURATION_FH
{
	unsigned long   Length;             // Length of structure
	unsigned long   HopPattern;         // As defined by 802.11, MSB set
	unsigned long   HopSet;             // to one if non-802.11
	unsigned long   DwellTime;          // units are Kusec
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION
{
	unsigned long   Length;             // Length of structure
	unsigned long   BeaconPeriod;       // units are Kusec
	unsigned long   ATIMWindow;         // units are Kusec
	unsigned long   DSConfig;           // Frequency, units are kHz
	NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef struct _RT_802_11_LINK_STATUS {
	unsigned long   CurrTxRate;         // in units of 0.5Mbps
	unsigned long   ChannelQuality;     // 0..100 %
	unsigned long   TxByteCount;        // both ok and fail
	unsigned long   RxByteCount;        // both ok and fail
	unsigned long   CentralChannel;     // 40MHz central channel number
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

typedef union _LARGE_INTEGER {
	struct {
		unsigned long LowPart;
		long HighPart;
	};
	struct {
		unsigned long LowPart;
		long HighPart;
	} u;
	signed long long QuadPart;
} LARGE_INTEGER;

typedef struct _NDIS_802_11_STATISTICS
{
	unsigned long   Length;             // Length of structure
	LARGE_INTEGER   TransmittedFragmentCount;
	LARGE_INTEGER   MulticastTransmittedFrameCount;
	LARGE_INTEGER   FailedCount;
	LARGE_INTEGER   RetryCount;
	LARGE_INTEGER   MultipleRetryCount;
	LARGE_INTEGER   RTSSuccessCount;
	LARGE_INTEGER   RTSFailureCount;
	LARGE_INTEGER   ACKFailureCount;
	LARGE_INTEGER   FrameDuplicateCount;
	LARGE_INTEGER   ReceivedFragmentCount;
	LARGE_INTEGER   MulticastReceivedFrameCount;
	LARGE_INTEGER   FCSErrorCount;
	LARGE_INTEGER   TransmittedFrameCount;
	LARGE_INTEGER   WEPUndecryptableCount;
	LARGE_INTEGER   TKIPLocalMICFailures;
	LARGE_INTEGER   TKIPRemoteMICErrors;
	LARGE_INTEGER   TKIPICVErrors;
	LARGE_INTEGER   TKIPCounterMeasuresInvoked;
	LARGE_INTEGER   TKIPReplays;
	LARGE_INTEGER   CCMPFormatErrors;
	LARGE_INTEGER   CCMPReplays;
	LARGE_INTEGER   CCMPDecryptErrors;
	LARGE_INTEGER   FourWayHandshakeFailures;
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;

//For SetBATable use
typedef struct {
	unsigned char   IsRecipient;
	unsigned char   MACAddr[ETH_ALEN];
	unsigned char   TID;
	unsigned char   BufSize;
	unsigned short  TimeOut;
	unsigned char   AllTid;  // If True, delete all TID for BA sessions with this MACaddr.
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

// Received Signal Strength Indication
typedef long NDIS_802_11_RSSI;           // in dBm

// Added new types for OFDM 5G and 2.4G
typedef enum _NDIS_802_11_NETWORK_TYPE
{
	Ndis802_11FH,
	Ndis802_11DS,
	Ndis802_11OFDM5,
	Ndis802_11OFDM24,
	Ndis802_11Automode,
	Ndis802_11OFDM5_N,
	Ndis802_11OFDM24_N,
	Ndis802_11OFDM5_AC,
	Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
	Ndis802_11IBSS,
	Ndis802_11Infrastructure,
	Ndis802_11AutoUnknown,
	Ndis802_11Monitor,
	Ndis802_11InfrastructureMax         // Not a real value, defined as upper bound
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

typedef unsigned char  NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];        // Set of 8 data rates
typedef unsigned char  NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];  // Set of 16 data rates

typedef struct PACKED _NDIS_WLAN_BSSID
{
	unsigned long                       Length;             // Length of this structure
	unsigned char                       MacAddress[ETH_ALEN];      // BSSID
	unsigned char                       Reserved[2];
	NDIS_802_11_SSID                    Ssid;               // SSID
	unsigned long                       Privacy;            // WEP encryption requirement
	NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
	NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
	NDIS_802_11_CONFIGURATION           Configuration;
	NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
	NDIS_802_11_RATES                   SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

// Added Capabilities, IELength and IEs for each BSSID
typedef struct PACKED _NDIS_WLAN_BSSID_EX
{
	unsigned long                       Length;             // Length of this structure
	unsigned char                       MacAddress[ETH_ALEN];      // BSSID
	unsigned char                       Reserved[2];
	NDIS_802_11_SSID                    Ssid;               // SSID
	unsigned int						Privacy;            // WEP encryption requirement
	NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
	NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
	NDIS_802_11_CONFIGURATION           Configuration;
	NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
	NDIS_802_11_RATES_EX                SupportedRates;
	unsigned long                       IELength;
	unsigned char                       IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

typedef struct PACKED _NDIS_802_11_BSSID_LIST_EX
{
	unsigned int            NumberOfItems;      // in list below, at least 1
	NDIS_WLAN_BSSID_EX      Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

typedef struct PACKED _NDIS_802_11_FIXED_IEs
{
	unsigned char   Timestamp[8];
	unsigned short  BeaconInterval;
	unsigned short  Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;

typedef struct PACKED _NDIS_802_11_VARIABLE_IEs
{
	unsigned char   ElementID;
	unsigned char   Length;        // Number of bytes in data field
	unsigned char   data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST
{
	unsigned int    NumberOfItems;  // in list below, at least 1
	NDIS_802_11_NETWORK_TYPE    NetworkType [1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

//For OID Query or Set about BA structure
typedef struct  _OID_BACAP_STRUC {
	unsigned char   RxBAWinLimit;
	unsigned char   TxBAWinLimit;
	unsigned char   Policy;     // 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid
	unsigned char   MpduDensity;// 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid
	unsigned char   AmsduEnable;//Enable AMSDU transmisstion
	unsigned char   AmsduSize;  // 0:3839, 1:7935 bytes. unsigned int  MSDUSizeToBytes[]    = { 3839, 7935};
	unsigned char   MMPSmode;   // MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable
	unsigned char   AutoBA;     // Auto BA will automatically , BOOLEAN
} OID_BACAP_STRUC, *POID_BACAP_STRUC;

typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
	Ndis802_11AuthModeOpen,
	Ndis802_11AuthModeShared,
	Ndis802_11AuthModeAutoSwitch,
	Ndis802_11AuthModeWPA,
	Ndis802_11AuthModeWPAPSK,
	Ndis802_11AuthModeWPANone,
	Ndis802_11AuthModeWPA2,
	Ndis802_11AuthModeWPA2PSK,
	Ndis802_11AuthModeWPA1WPA2,
	Ndis802_11AuthModeWPA1PSKWPA2PSK,
#ifdef WAPI_SUPPORT
	Ndis802_11AuthModeWAICERT,                      // WAI certificate authentication
	Ndis802_11AuthModeWAIPSK,                       // WAI pre-shared key
#endif // WAPI_SUPPORT //
	Ndis802_11AuthModeMax           // Not a real mode, defined as upper bound
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef enum _NDIS_802_11_WEP_STATUS
{
	Ndis802_11WEPEnabled,
	Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
	Ndis802_11WEPDisabled,
	Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
	Ndis802_11WEPKeyAbsent,
	Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
	Ndis802_11WEPNotSupported,
	Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
	Ndis802_11Encryption2Enabled,
	Ndis802_11Encryption2KeyAbsent,
	Ndis802_11Encryption3Enabled,
	Ndis802_11Encryption3KeyAbsent
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS,
  NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef enum _NDIS_802_11_POWER_MODE
{
	Ndis802_11PowerModeCAM,
	Ndis802_11PowerModeMAX_PSP,
	Ndis802_11PowerModeFast_PSP,
	Ndis802_11PowerModeMax      // not a real mode, defined as an upper bound
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef enum _RT_802_11_PREAMBLE {
	Rt802_11PreambleLong,
	Rt802_11PreambleShort,
	Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;

typedef struct _RT_PROFILE_SETTING {
	unsigned int                        ProfileDataType; //0x18140201
	unsigned char                       Profile[32+1];
	unsigned char                       SSID[NDIS_802_11_LENGTH_SSID+1];
	unsigned int                        SsidLen;
	unsigned int                        Channel;
	NDIS_802_11_AUTHENTICATION_MODE     Authentication; //Ndis802_11AuthModeOpen, Ndis802_11AuthModeShared, Ndis802_11AuthModeWPAPSK
	NDIS_802_11_WEP_STATUS              Encryption; //Ndis802_11WEPEnabled, Ndis802_11WEPDisabled, Ndis802_11Encryption2Enabled, Ndis802_11Encryption3Enabled
	NDIS_802_11_NETWORK_INFRASTRUCTURE  NetworkType;
	unsigned int                        KeyDefaultId;
	unsigned int                        Key1Type;
	unsigned int                        Key2Type;
	unsigned int                        Key3Type;
	unsigned int                        Key4Type;
	unsigned int                        Key1Length;
	unsigned int                        Key2Length;
	unsigned int                        Key3Length;
	unsigned int                        Key4Length;
	char                                Key1[26+1];
	char                                Key2[26+1];
	char                                Key3[26+1];
	char                                Key4[26+1];
	char                                WpaPsk[64+1]; //[32+1];
	unsigned int                        TransRate;
	unsigned int                        TransPower;
	char                                RTSCheck;  //boolean
	unsigned int                        RTS;
	char                                FragmentCheck; //boolean
	unsigned int                        Fragment;
	NDIS_802_11_POWER_MODE              PSmode;
	RT_802_11_PREAMBLE                  PreamType;
	unsigned int                        AntennaRx;
	unsigned int                        AntennaTx;
	unsigned int                        CountryRegion;
	//Advance
	unsigned int                        AdhocMode;
	unsigned int                        Active; // 0 is the profile is set as connection profile, 1 is not.
	struct  _RT_PROFILE_SETTING         *Next;
} RT_PROFILE_SETTING, *PRT_PROFILE_SETTING;

typedef struct _NDIS_802_11_REMOVE_KEY
{
	unsigned int        Length;             // Length of this structure
	unsigned int        KeyIndex;
	unsigned char       BSSID[ETH_ALEN];
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

// Key mapping keys require a BSSID
typedef struct _NDIS_802_11_KEY
{
    unsigned int        Length;             // Length of this structure
	unsigned int        KeyIndex;
	unsigned int        KeyLength;          // length of key in bytes
	unsigned char       BSSID[ETH_ALEN];
	NDIS_802_11_KEY_RSC KeyRSC;
	unsigned char       KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_KEY, *PNDIS_802_11_KEY;

typedef struct _NDIS_802_11_WEP                                                                                                                              {
	unsigned int        Length;        // Length of this structure
	unsigned int        KeyIndex;      // 0 is the per-client key, 1-N are the global keys
	unsigned int        KeyLength;     // length of key in bytes
	unsigned char       KeyMaterial[1];// variable length depending on above field
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;

typedef struct _NDIS_802_11_PASSPHRASE
{
	unsigned int			KeyLength;          // length of key in bytes
	unsigned char			BSSID[ETH_ALEN];
	unsigned char           KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_PASSPHRASE, *PNDIS_802_11_PASSPHRASE;


/* MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!! */
typedef union _MACHTTRANSMIT_SETTING {
	struct  {
		unsigned short MCS:6;		/* MCS */
		unsigned short ldpc:1;
		unsigned short BW:2;		/* channel bandwidth 20MHz/40/80 MHz */
		unsigned short ShortGI:1;
		unsigned short STBC:1;		/* only support in HT/VHT mode with MCS0~7 */
		unsigned short eTxBF:1;
		unsigned short iTxBF:1;
		unsigned short MODE:3;		/* Use definition MODE_xxx. */
	} field;
	unsigned short      word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char	ApIdx;
	unsigned char	Addr[ETH_ALEN];
	unsigned char	Aid;
	unsigned char	Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char	MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char		AvgRssi0;
	char		AvgRssi1;
	char		AvgRssi2;
	unsigned long long	TxBytes;
	unsigned long long 	RxBytes;
	unsigned int	ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int	LastRxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
    unsigned long	Num;
    RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

typedef struct _PAIR_CHANNEL_FREQ_ENTRY
{
	unsigned long   lChannel;
	unsigned long   lFreq;
} PAIR_CHANNEL_FREQ_ENTRY, *PPAIR_CHANNEL_FREQ_ENTRY;

struct WLAN_AP_ENTRY {
	unsigned char chan;
	char ssid[33];
	unsigned char bssid[6];
	char security[23];
	unsigned char signal_percent;
	char wmode[9];
	char extch[7];
	char nt[4];
};

#endif
