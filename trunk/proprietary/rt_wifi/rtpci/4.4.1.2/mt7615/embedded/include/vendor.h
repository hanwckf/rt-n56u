
struct _RTMP_ADAPTER;

#define RALINK_IE_LEN   0x9
#define MEDIATEK_IE_LEN 0x9

#define RALINK_AGG_CAP      (1 << 0)
#define RALINK_PIGGY_CAP    (1 << 1)
#define RALINK_RDG_CAP      (1 << 2)
#define RALINK_256QAM_CAP   (1 << 3)

#define MEDIATEK_256QAM_CAP (1 << 3)

#define BROADCOM_256QAM_CAP (1 << 0)
#define BROADCOM_2G_4SS_CAP (1 << 4)


typedef struct GNU_PACKED _ie_hdr {
    UCHAR eid;
    UINT8 len;
} IE_HEADER;


struct GNU_PACKED _ralink_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR cap0;
    UCHAR cap1;
    UCHAR cap2;
    UCHAR cap3;
};


typedef struct GNU_PACKED _vht_cap_ie {
    IE_HEADER ie_hdr;
    UCHAR vht_cap_info[4];
    UCHAR support_vht_mcs_nss[8];
} VHT_CAP;


typedef struct GNU_PACKED _vht_op_ie {
    IE_HEADER ie_hdr;
    UCHAR vht_op_info[3];
    UCHAR basic_vht_mcs_nss[2];
} VHT_OP;


typedef struct GNU_PACKED _vht_tx_pwr_env_ie {
    IE_HEADER ie_hdr;
    UCHAR tx_pwr_info;
    UCHAR local_max_txpwr_20Mhz;
    UCHAR local_max_txpwr_40Mhz;
} VHT_TX_PWR_ENV;


struct GNU_PACKED _mediatek_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR cap0;
    UCHAR cap1;
    UCHAR cap2;
    UCHAR cap3;
};


struct GNU_PACKED _mediatek_vht_ie {
    VHT_CAP vht_cap;
    VHT_OP vht_op;
    VHT_TX_PWR_ENV vht_txpwr_env;
};


struct GNU_PACKED _broadcom_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR fixed_pattern[2];
    VHT_CAP vht_cap;
    VHT_OP vht_op;
    VHT_TX_PWR_ENV vht_txpwr_env;
};


ULONG build_vendor_ie(struct _RTMP_ADAPTER *pAd, 
        struct wifi_dev *wdev, UCHAR *frame_buffer);

VOID check_vendor_ie(struct _RTMP_ADAPTER *pAd,
        UCHAR *ie_buffer, struct _vendor_ie_cap *vendor_ie);
