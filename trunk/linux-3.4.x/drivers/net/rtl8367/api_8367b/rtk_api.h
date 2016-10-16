/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 28599 $
 * $Date: 2012-05-07 09:41:37 +0800 (星期一, 07 五月 2012) $
 *
 * Purpose : Definition function prototype of RTK API.
 *
 * Feature : Function prototype definition
 *
 */

#ifndef __RTK_API_H__
#define __RTK_API_H__

/*
 * Include Files
 */
#include "rtk_types.h"
#include "rtk_error.h"

/*
 * Data Type Declaration
 */
#define ENABLE                                      1
#define DISABLE                                     0

#define PHY_CONTROL_REG                             0
#define PHY_STATUS_REG                              1
#define PHY_AN_ADVERTISEMENT_REG                    4
#define PHY_AN_LINKPARTNER_REG                      5
#define PHY_1000_BASET_CONTROL_REG                  9
#define PHY_1000_BASET_STATUS_REG                   10
#define PHY_RESOLVED_REG                            17

/*Qos related configuration define*/
#define QOS_DEFAULT_TICK_PERIOD                     (19-1)
#define QOS_DEFAULT_BYTE_PER_TOKEN                  34
#define QOS_DEFAULT_LK_THRESHOLD                    (34*3) /* Why use 0x400? */


#define QOS_DEFAULT_INGRESS_BANDWIDTH               0x3FFF /* 0x3FFF => unlimit */
#define QOS_DEFAULT_EGRESS_BANDWIDTH                0x3D08 /*( 0x3D08 + 1) * 64Kbps => 1Gbps*/
#define QOS_DEFAULT_PREIFP                          1
#define QOS_DEFAULT_PACKET_USED_PAGES_FC            0x60
#define QOS_DEFAULT_PACKET_USED_FC_EN               0
#define QOS_DEFAULT_QUEUE_BASED_FC_EN               1

#define QOS_DEFAULT_PRIORITY_SELECT_PORT            8
#define QOS_DEFAULT_PRIORITY_SELECT_1Q              0
#define QOS_DEFAULT_PRIORITY_SELECT_ACL             0
#define QOS_DEFAULT_PRIORITY_SELECT_DSCP            0

#define QOS_DEFAULT_DSCP_MAPPING_PRIORITY           0

#define QOS_DEFAULT_1Q_REMARKING_ABILITY            0
#define QOS_DEFAULT_DSCP_REMARKING_ABILITY          0
#define QOS_DEFAULT_QUEUE_GAP                       20
#define QOS_DEFAULT_QUEUE_NO_MAX                    6
#define QOS_DEFAULT_AVERAGE_PACKET_RATE             0x3FFF
#define QOS_DEFAULT_BURST_SIZE_IN_APR               0x3F
#define QOS_DEFAULT_PEAK_PACKET_RATE                2
#define QOS_DEFAULT_SCHEDULER_ABILITY_APR           1     /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_PPR           1    /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_WFQ           1    /*disable*/

#define QOS_WEIGHT_MAX                              128

#define LED_GROUP_MAX                               3

#define RTK_FILTER_RAW_FIELD_NUMBER                8

#define ACL_DEFAULT_ABILITY                         0
#define ACL_DEFAULT_UNMATCH_PERMIT                  1

#define ACL_RULE_FREE                               0
#define ACL_RULE_INAVAILABLE                        1
#define ACL_RULE_CARETAG_MASK						0x1F
#define FILTER_POLICING_MAX                         4
#define FILTER_LOGGING_MAX                          8
#define FILTER_PATTERN_MAX                          4

#define STORM_UNUC_INDEX                            28
#define STORM_UNMC_INDEX                            29
#define STORM_MC_INDEX                              30
#define STORM_BC_INDEX                              31

#define RTK_MAX_NUM_OF_INTERRUPT_TYPE               2
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1
#define RTK_MAX_NUM_OF_PRIORITY                     8
#define RTK_MAX_NUM_OF_QUEUE                        8
#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               1
#define RTK_MAX_NUM_OF_PORT                         8
#define RTK_PORT_ID_MAX                             (RTK_MAX_NUM_OF_PORT-1)
#define RTK_PHY_ID_MAX                              (RTK_MAX_NUM_OF_PORT-4)
#define RTK_PORT_COMBO_ID                           4
#define RTK_MAX_NUM_OF_PROTO_TYPE                   0xFFFF
#define RTK_MAX_NUM_OF_MSTI                         0xF
#define RTK_MAX_NUM_OF_LEARN_LIMIT                  0x840
#define RTK_MAX_PORT_MASK                           0xFF
#define RTK_EFID_MAX                                0x7
#define RTK_FID_MAX                                 0xF
#define RTK_MAX_NUM_OF_FILTER_TYPE                  5
#define RTK_MAX_NUM_OF_FILTER_FIELD                 8

#define RTK_FIBER_FORCE_1000M                       3
#define RTK_FIBER_FORCE_100M                        5
#define RTK_FIBER_FORCE_100M1000M                   7

#define RTK_MAX_NUM_OF_METER                        32

#define RTK_FLOWCTRL_PAUSE_ALL                      1980
#define RTK_FLOWCTRL_DROP_ALL                       2012
#define RTK_FLOWCTRL_PAUSE_SYSTEM_ON                1200
#define RTK_FLOWCTRL_PAUSE_SYSTEM_OFF               1000
#define RTK_FLOWCTRL_DROP_SYSTEM_ON                 1200
#define RTK_FLOWCTRL_DROP_SYSTEM_OFF                1000
#define RTK_FLOWCTRL_PAUSE_SHARE_ON                 216
#define RTK_FLOWCTRL_PAUSE_SHARE_OFF                208
#define RTK_FLOWCTRL_DROP_SHARE_ON                  216
#define RTK_FLOWCTRL_DROP_SHARE_OFF                 208
#define RTK_FLOWCTRL_PAUSE_PORT_ON                  140
#define RTK_FLOWCTRL_PAUSE_PORT_OFF                 132
#define RTK_FLOWCTRL_DROP_PORT_ON                   140
#define RTK_FLOWCTRL_DROP_PORT_OFF                  132
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_ON          26
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_OFF         22
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_ON           26
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_OFF          22
#define RTK_FLOWCTRL_PORT_DROP_EGRESS               210
#define RTK_FLOWCTRL_QUEUE_DROP_EGRESS              2047
#define RTK_FLOWCTRL_PORT_GAP                       72
#define RTK_FLOWCTRL_QUEUE_GAP                      18

#define RTK_WHOLE_SYSTEM                            0xFF

#define RTK_EXT_0                                   0
#define RTK_EXT_1                                   1
#define RTK_EXT_2                                   2

#define RTK_EXT_0_MAC                               5
#define RTK_EXT_1_MAC                               6
#define RTK_EXT_2_MAC                               7

#define RTK_MAC_ADDR_LEN                            6
#define RTK_MAX_LUT_ADDRESS                         (2048+64)
#define RTK_MAX_LUT_ADDR_ID                         (RTK_MAX_LUT_ADDRESS - 1)

#define IPV6_ADDR_LEN                               16
#define IPV4_ADDR_LEN                               4

#define RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH   3UL

#define RTK_IPV6_ADDR_WORD_LENGTH                   4UL

#define RTK_FILTER_FIELD_USED_MAX					8
#define RTK_FILTER_FIELD_INDEX(template, index)  	((template << 4) + index)

#define ALLPORT 0xFF

#define RTK_INDRECT_ACCESS_CRTL                     0x1f00
#define RTK_INDRECT_ACCESS_STATUS                   0x1f01
#define RTK_INDRECT_ACCESS_ADDRESS                  0x1f02
#define RTK_INDRECT_ACCESS_WRITE_DATA               0x1f03
#define RTK_INDRECT_ACCESS_READ_DATA                0x1f04
#define RTK_INDRECT_ACCESS_DELAY                    0x1f80
#define RTK_INDRECT_ACCESS_BURST                    0x1f81
#define RTK_RW_MASK                                 0x2
#define RTK_CMD_MASK                                0x1
#define RTK_PHY_BUSY_OFFSET                         2

#define RTK_IVL_MODE_FID                            0xFFFF

typedef enum rtk_cpu_insert_e
{
    CPU_INSERT_TO_ALL = 0,
    CPU_INSERT_TO_TRAPPING,
    CPU_INSERT_TO_NONE,
    CPU_INSERT_END
}rtk_cpu_insert_t;

typedef enum rtk_cpu_position_e
{
    CPU_POS_ATTER_DA = 0,
    CPU_POS_AFTER_CRC,
    CPU_POS_END
}rtk_cpu_position_t;

typedef rtk_uint32  rtk_data_t;

/* Type of port-based dot1x auth/unauth*/
typedef enum rtk_dot1x_auth_status_e
{
    UNAUTH = 0,
    AUTH,
    AUTH_STATUS_END
} rtk_dot1x_auth_status_t;

typedef enum rtk_dot1x_direction_e
{
    DIR_BOTH = 0,
    DIR_IN,
    DIRECTION_END
} rtk_dot1x_direction_t;

typedef enum rtk_mode_ext_e
{
    MODE_EXT_DISABLE = 0,
    MODE_EXT_RGMII,
    MODE_EXT_MII_MAC,
    MODE_EXT_MII_PHY,
    MODE_EXT_TMII_MAC,
    MODE_EXT_TMII_PHY,
    MODE_EXT_GMII,
    MODE_EXT_RMII_MAC,
    MODE_EXT_RMII_PHY,
    MODE_EXT_RGMII_33V,
    MODE_EXT_END
} rtk_mode_ext_t;

typedef struct
{
    rtk_uint32 value[RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH];
} rtk_filter_dot1as_timestamp_t;

/* unauth pkt action */
typedef enum rtk_dot1x_unauth_action_e
{
    DOT1X_ACTION_DROP = 0,
    DOT1X_ACTION_TRAP2CPU,
    DOT1X_ACTION_GUESTVLAN,
    DOT1X_ACTION_END
} rtk_dot1x_unauth_action_t;

typedef rtk_uint32  rtk_dscp_t;         /* dscp vlaue */

typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef rtk_uint32  rtk_fid_t;        /* filter id type */


/* ethernet address type */
typedef struct  rtk_mac_s
{
    rtk_uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;


typedef enum rtk_filter_act_enable_e
{
    /* CVLAN */
    FILTER_ENACT_INGRESS_CVLAN_INDEX = 0,
    FILTER_ENACT_INGRESS_CVLAN_VID,
    FILTER_ENACT_CVLAN_INGRESS,
    FILTER_ENACT_CVLAN_EGRESS,
    FILTER_ENACT_CVLAN_SVID,
    FILTER_ENACT_POLICING_1,

    /* SVLAN */
    FILTER_ENACT_EGRESS_SVLAN_INDEX,
    FILTER_ENACT_SVLAN_INGRESS,
    FILTER_ENACT_SVLAN_EGRESS,
    FILTER_ENACT_SVLAN_CVID,
    FILTER_ENACT_POLICING_2,

    /* Policing and Logging */
    FILTER_ENACT_POLICING_0,

    /* Forward */
    FILTER_ENACT_COPY_CPU,
    FILTER_ENACT_DROP,
    FILTER_ENACT_ADD_DSTPORT,
    FILTER_ENACT_REDIRECT,
    FILTER_ENACT_MIRROR,
    FILTER_ENACT_TRAP_CPU,

    /* QoS */
    FILTER_ENACT_PRIORITY,
    FILTER_ENACT_DSCP_REMARK,
    FILTER_ENACT_1P_REMARK,
    FILTER_ENACT_POLICING_3,

    /* Interrutp and GPO */
    FILTER_ENACT_INTERRUPT,
    FILTER_ENACT_GPO,

    FILTER_ENACT_END,
} rtk_filter_act_enable_t;


#define FILTER_ENACT_CVLAN_MASK         0x01
#define FILTER_ENACT_SVLAN_MASK         0x02
#define FILTER_ENACT_PRIORITY_MASK    	0x04
#define FILTER_ENACT_POLICING_MASK    	0x08
#define FILTER_ENACT_FWD_MASK    		0x10
#define FILTER_ENACT_INTGPIO_MASK    	0x20
#define FILTER_ENACT_INIT_MASK			0x3F

#define FILTER_ENACT_CVLAN_TYPE(type)	(type - FILTER_ENACT_CVLAN_INGRESS)
#define FILTER_ENACT_SVLAN_TYPE(type)	(type - FILTER_ENACT_SVLAN_INGRESS)
#define FILTER_ENACT_FWD_TYPE(type)		(type - FILTER_ENACT_ADD_DSTPORT)
#define FILTER_ENACT_PRI_TYPE(type)		(type - FILTER_ENACT_PRIORITY)

typedef struct
{
    rtk_filter_act_enable_t actEnable[FILTER_ENACT_END];

	/* CVLAN acton */
	rtk_uint32 		filterIngressCvlanIdx;
	rtk_uint32     	filterIngressCvlanVid;
	rtk_uint32 		filterCvlanIdx;
	/* SVLAN action */
	rtk_uint32     	filterEgressSvlanIdx;
    rtk_uint32      filterSvlanVid;
	rtk_uint32 		filterSvlanIdx;

	/* Policing action */
	rtk_uint32     	filterPolicingIdx[FILTER_POLICING_MAX];

	/* Forwarding action */
    rtk_uint32      filterRedirectPortmask;
    rtk_uint32      filterAddDstPortmask;
	rtk_uint32 		filterPortmask;

	/* QOS action */
    rtk_uint32      filterPriority;

	/*GPO*/
    rtk_uint32      filterPin;

} rtk_filter_action_t;

typedef struct rtk_filter_flag_s
{
    rtk_uint32 value;
    rtk_uint32 mask;
} rtk_filter_flag_t;

typedef enum rtk_filter_care_tag_index_e
{
    CARE_TAG_CTAG = 0,
    CARE_TAG_STAG,
    CARE_TAG_PPPOE,
    CARE_TAG_IPV4,
    CARE_TAG_IPV6,
    CARE_TAG_TCP,
    CARE_TAG_UDP,
    CARE_TAG_ARP,
    CARE_TAG_RSV1,
    CARE_TAG_RSV2,
    CARE_TAG_ICMP,
    CARE_TAG_IGMP,
    CARE_TAG_LLC,
    CARE_TAG_RSV3,
    CARE_TAG_HTTP,
    CARE_TAG_RSV4,
    CARE_TAG_RSV5,
    CARE_TAG_DHCP,
    CARE_TAG_DHCPV6,
    CARE_TAG_SNMP,
    CARE_TAG_OAM,
    CARE_TAG_END,
} rtk_filter_care_tag_index_t;

typedef struct rtk_filter_care_tag_s
{
    rtk_filter_flag_t tagType[CARE_TAG_END];
} rtk_filter_care_tag_t;

typedef struct rtk_filter_field rtk_filter_field_t;

typedef enum rtk_filter_field_data_type_e
{
    FILTER_FIELD_DATA_MASK = 0,
    FILTER_FIELD_DATA_RANGE,
    FILTER_FIELD_DATA_END ,
} rtk_filter_field_data_type_t;

typedef struct rtk_filter_ip_s
{
    rtk_uint32 dataType;
    rtk_uint32 rangeStart;
    rtk_uint32 rangeEnd;
    rtk_uint32 value;
    rtk_uint32 mask;
} rtk_filter_ip_t;

typedef struct rtk_filter_mac_s
{
    rtk_uint32 dataType;
    rtk_mac_t value;
    rtk_mac_t mask;
    rtk_mac_t rangeStart;
    rtk_mac_t rangeEnd;
} rtk_filter_mac_t;

typedef rtk_uint32 rtk_filter_op_t;

typedef struct rtk_filter_value_s
{
    rtk_uint32 dataType;
    rtk_uint32 value;
    rtk_uint32 mask;
    rtk_uint32 rangeStart;
    rtk_uint32 rangeEnd;
} rtk_filter_value_t;

typedef struct rtk_filter_tag_s
{
    rtk_filter_value_t pri;
    rtk_filter_flag_t cfi;
    rtk_filter_value_t vid;
} rtk_filter_tag_t;

typedef struct rtk_filter_ipFlag_s
{
    rtk_filter_flag_t xf;
    rtk_filter_flag_t mf;
    rtk_filter_flag_t df;
} rtk_filter_ipFlag_t;

typedef struct
{
    rtk_uint32 addr[RTK_IPV6_ADDR_WORD_LENGTH];
} rtk_filter_ip6_addr_t;

typedef struct
{
    rtk_uint32 dataType;
    rtk_filter_ip6_addr_t value;
    rtk_filter_ip6_addr_t mask;
    rtk_filter_ip6_addr_t rangeStart;
    rtk_filter_ip6_addr_t rangeEnd;
} rtk_filter_ip6_t;

typedef rtk_uint32 rtk_filter_number_t;

typedef struct rtk_filter_pattern_s
{
    rtk_uint32 value[FILTER_PATTERN_MAX];
    rtk_uint32 mask[FILTER_PATTERN_MAX];
} rtk_filter_pattern_t;

typedef struct rtk_filter_tcpFlag_s
{
    rtk_filter_flag_t urg;
    rtk_filter_flag_t ack;
    rtk_filter_flag_t psh;
    rtk_filter_flag_t rst;
    rtk_filter_flag_t syn;
    rtk_filter_flag_t fin;
    rtk_filter_flag_t ns;
    rtk_filter_flag_t cwr;
    rtk_filter_flag_t ece;
} rtk_filter_tcpFlag_t;

typedef rtk_uint32 rtk_filter_field_raw_t;

typedef enum rtk_filter_field_temple_input_e
{
    FILTER_FIELD_TEMPLE_INPUT_TYPE = 0,
    FILTER_FIELD_TEMPLE_INPUT_INDEX,
    FILTER_FIELD_TEMPLE_INPUT_MAX ,
} rtk_filter_field_temple_input_t;

struct rtk_filter_field
{
    rtk_uint32 fieldType;

    union
    {
        /* L2 struct */
        rtk_filter_mac_t       dmac;
        rtk_filter_mac_t       smac;
        rtk_filter_value_t     etherType;
        rtk_filter_tag_t       ctag;
        rtk_filter_tag_t       relayCtag;
        rtk_filter_tag_t       stag;
        rtk_filter_tag_t       l2tag;
        rtk_filter_dot1as_timestamp_t dot1asTimeStamp;
        rtk_filter_mac_t       mac;

        /* L3 struct */
	    rtk_filter_ip_t      sip;
        rtk_filter_ip_t      dip;
        rtk_filter_ip_t      ip;
        rtk_filter_value_t   protocol;
        rtk_filter_value_t   ipTos;
        rtk_filter_ipFlag_t  ipFlag;
        rtk_filter_value_t   ipOffset;
	    rtk_filter_ip6_t     sipv6;
        rtk_filter_ip6_t     dipv6;
        rtk_filter_ip6_t     ipv6;
        rtk_filter_value_t   ipv6TrafficClass;
        rtk_filter_value_t   ipv6NextHeader;
        rtk_filter_value_t   flowLabel;

        /* L4 struct */
        rtk_filter_value_t   tcpSrcPort;
        rtk_filter_value_t   tcpDstPort;
        rtk_filter_tcpFlag_t tcpFlag;
        rtk_filter_value_t   tcpSeqNumber;
        rtk_filter_value_t   tcpAckNumber;
        rtk_filter_value_t   udpSrcPort;
        rtk_filter_value_t   udpDstPort;
        rtk_filter_value_t   icmpCode;
        rtk_filter_value_t   icmpType;
        rtk_filter_value_t   igmpType;

        /* pattern match */
        rtk_filter_pattern_t pattern;

        rtk_filter_value_t   inData;

	} filter_pattern_union;

    rtk_uint32 fieldTemplateNo;
    rtk_uint32 fieldTemplateIdx[RTK_FILTER_FIELD_USED_MAX];

    struct rtk_filter_field *next;
};

typedef enum rtk_filter_field_type_e
{
    FILTER_FIELD_DMAC = 0,
    FILTER_FIELD_SMAC,
    FILTER_FIELD_ETHERTYPE,
    FILTER_FIELD_CTAG,
    FILTER_FIELD_STAG,

    FILTER_FIELD_IPV4_SIP,
    FILTER_FIELD_IPV4_DIP,
    FILTER_FIELD_IPV4_TOS,
    FILTER_FIELD_IPV4_PROTOCOL,
    FILTER_FIELD_IPV4_FLAG,
    FILTER_FIELD_IPV4_OFFSET,
    FILTER_FIELD_IPV6_SIPV6,
    FILTER_FIELD_IPV6_DIPV6,
    FILTER_FIELD_IPV6_TRAFFIC_CLASS,
    FILTER_FIELD_IPV6_NEXT_HEADER,

    FILTER_FIELD_TCP_SPORT,
    FILTER_FIELD_TCP_DPORT,
    FILTER_FIELD_TCP_FLAG,
    FILTER_FIELD_UDP_SPORT,
    FILTER_FIELD_UDP_DPORT,
    FILTER_FIELD_ICMP_CODE,
    FILTER_FIELD_ICMP_TYPE,
    FILTER_FIELD_IGMP_TYPE,

    FILTER_FIELD_VID_RANGE,
    FILTER_FIELD_IP_RANGE,
    FILTER_FIELD_PORT_RANGE,

    FILTER_FIELD_USER_DEFINED00,
    FILTER_FIELD_USER_DEFINED01,
    FILTER_FIELD_USER_DEFINED02,
    FILTER_FIELD_USER_DEFINED03,
    FILTER_FIELD_USER_DEFINED04,
    FILTER_FIELD_USER_DEFINED05,
    FILTER_FIELD_USER_DEFINED06,
    FILTER_FIELD_USER_DEFINED07,
    FILTER_FIELD_USER_DEFINED08,
    FILTER_FIELD_USER_DEFINED09,
    FILTER_FIELD_USER_DEFINED10,
    FILTER_FIELD_USER_DEFINED11,
    FILTER_FIELD_USER_DEFINED12,
    FILTER_FIELD_USER_DEFINED13,
    FILTER_FIELD_USER_DEFINED14,
    FILTER_FIELD_USER_DEFINED15,

    FILTER_FIELD_PATTERN_MATCH,

    FILTER_FIELD_END,
} rtk_filter_field_type_t;

typedef enum rtk_filter_field_type_raw_e
{
    FILTER_FIELD_RAW_UNUSED = 0,
    FILTER_FIELD_RAW_DMAC_15_0,
    FILTER_FIELD_RAW_DMAC_31_16,
	FILTER_FIELD_RAW_DMAC_47_32,
    FILTER_FIELD_RAW_SMAC_15_0,
    FILTER_FIELD_RAW_SMAC_31_16,
    FILTER_FIELD_RAW_SMAC_47_32,
    FILTER_FIELD_RAW_ETHERTYPE,
    FILTER_FIELD_RAW_STAG,
    FILTER_FIELD_RAW_CTAG,

    FILTER_FIELD_RAW_IPV4_SIP_15_0 = 0x10,
    FILTER_FIELD_RAW_IPV4_SIP_31_16,
    FILTER_FIELD_RAW_IPV4_DIP_15_0,
    FILTER_FIELD_RAW_IPV4_DIP_31_16,


    FILTER_FIELD_RAW_IPV6_SIP_15_0 = 0x20,
    FILTER_FIELD_RAW_IPV6_SIP_31_16,
    FILTER_FIELD_RAW_IPV6_DIP_15_0,
    FILTER_FIELD_RAW_IPV6_DIP_31_16,

	FILTER_FIELD_RAW_VIDRANGE = 0x30,
	FILTER_FIELD_RAW_IPRANGE,
	FILTER_FIELD_RAW_PORTRANGE,

	FILTER_FIELD_RAW_FIELD_SELECT00 = 0x40,
	FILTER_FIELD_RAW_FIELD_SELECT01,
	FILTER_FIELD_RAW_FIELD_SELECT02,
	FILTER_FIELD_RAW_FIELD_SELECT03,
	FILTER_FIELD_RAW_FIELD_SELECT04,
	FILTER_FIELD_RAW_FIELD_SELECT05,
	FILTER_FIELD_RAW_FIELD_SELECT06,
	FILTER_FIELD_RAW_FIELD_SELECT07,
	FILTER_FIELD_RAW_FIELD_SELECT08,
	FILTER_FIELD_RAW_FIELD_SELECT09,
	FILTER_FIELD_RAW_FIELD_SELECT10,
	FILTER_FIELD_RAW_FIELD_SELECT11,
	FILTER_FIELD_RAW_FIELD_SELECT12,
	FILTER_FIELD_RAW_FIELD_SELECT13,
	FILTER_FIELD_RAW_FIELD_SELECT14,
	FILTER_FIELD_RAW_FIELD_SELECT15,

    FILTER_FIELD_RAW_END,
} rtk_filter_field_type_raw_t;



typedef enum rtk_filter_flag_care_type_e
{
    FILTER_FLAG_CARE_DONT_CARE = 0,
    FILTER_FLAG_CARE_1,
    FILTER_FLAG_CARE_0,
    FILTER_FLAG_END
} rtk_filter_flag_care_type_t;

typedef rtk_uint32  rtk_filter_id_t;    /* filter id type */

typedef enum rtk_filter_invert_e
{
    FILTER_INVERT_DISABLE = 0,
    FILTER_INVERT_ENABLE,
    FILTER_INVERT_END,
} rtk_filter_invert_t;

typedef rtk_uint32 rtk_filter_state_t;

typedef rtk_uint32 rtk_filter_unmatch_action_t;

typedef enum rtk_filter_unmatch_action_e
{
    FILTER_UNMATCH_DROP = 0,
    FILTER_UNMATCH_PERMIT,
    FILTER_UNMATCH_END,
} rtk_filter_unmatch_action_type_t;

typedef struct
{
    rtk_filter_field_t      *fieldHead;
    rtk_filter_care_tag_t   careTag;
    rtk_filter_value_t      activeport;

    rtk_filter_invert_t     invert;
} rtk_filter_cfg_t;

typedef struct
{
    rtk_filter_field_raw_t      dataFieldRaw[RTK_FILTER_RAW_FIELD_NUMBER];
    rtk_filter_field_raw_t      careFieldRaw[RTK_FILTER_RAW_FIELD_NUMBER];
	rtk_filter_field_type_raw_t fieldRawType[RTK_FILTER_RAW_FIELD_NUMBER];
    rtk_filter_care_tag_t       careTag;
    rtk_filter_value_t          activeport;

    rtk_filter_invert_t         invert;
	rtk_enable_t                valid;
} rtk_filter_cfg_raw_t;

typedef struct
{
    rtk_uint32 index;
    rtk_filter_field_type_raw_t fieldType[RTK_FILTER_RAW_FIELD_NUMBER];
} rtk_filter_template_t;

typedef enum rtk_field_sel_e
{
    FORMAT_DEFAULT = 0,
    FORMAT_RAW,
	FORMAT_LLC,
	FORMAT_IPV4,
	FORMAT_ARP,
	FORMAT_IPV6,
	FORMAT_IPPAYLOAD,
	FORMAT_L4PAYLOAD,
    FORMAT_END
}rtk_field_sel_t;

typedef enum rtk_filter_iprange_e
{
    IPRANGE_UNUSED = 0,
    IPRANGE_IPV4_SIP,
    IPRANGE_IPV4_DIP,
    IPRANGE_IPV6_SIP,
    IPRANGE_IPV6_DIP,
    IPRANGE_END
}rtk_filter_iprange_t;

typedef enum rtk_filter_vidrange_e
{
    VIDRANGE_UNUSED = 0,
    VIDRANGE_CVID,
    VIDRANGE_SVID,
    VIDRANGE_END
}rtk_filter_vidrange_t;

typedef enum rtk_filter_portrange_e
{
    PORTRANGE_UNUSED = 0,
    PORTRANGE_SPORT,
    PORTRANGE_DPORT,
    PORTRANGE_END
}rtk_filter_portrange_t;

typedef enum rtk_igmp_type_e
{
    IGMP_IPV4 = 0,
    IGMP_PPPOE_IPV4,
    IGMP_MLD,
    IGMP_PPPOE_MLD,
    IGMP_TYPE_END
} rtk_igmp_type_t;

typedef rtk_uint32 rtk_int_info_t;

typedef enum rtk_int_type_e
{
    INT_TYPE_LINK_STATUS = 0,
    INT_TYPE_METER_EXCEED,
    INT_TYPE_LEARN_LIMIT,
    INT_TYPE_LINK_SPEED,
    INT_TYPE_CONGEST,
    INT_TYPE_GREEN_FEATURE,
    INT_TYPE_LOOP_DETECT,
    INT_TYPE_8051,
    INT_TYPE_CABLE_DIAG,
    INT_TYPE_ACL,
    INT_TYPE_UPS,
    INT_TYPE_SLIENT,
    INT_TYPE_END
}rtk_int_type_t;

typedef enum rtk_int_advType_e
{
    ADV_L2_LEARN_PORT_MASK = 0,
    ADV_SPEED_CHANGE_PORT_MASK,
    ADV_SPECIAL_CONGESTION_PORT_MASK,
    ADV_PORT_LINKDOWN_PORT_MASK,
    ADV_PORT_LINKUP_PORT_MASK,
    ADV_METER0_15_MASK,
    ADV_METER16_31_MASK,
    ADV_METER32_47_MASK,
    ADV_METER48_63_MASK,
    ADV_RLDP_LOOPED,
	ADV_RLDP_RELEASED,
    ADV_END,
} rtk_int_advType_t;

typedef enum rtk_int_polarity_e
{
    INT_POLAR_HIGH = 0,
    INT_POLAR_LOW,
    INT_POLAR_END
} rtk_int_polarity_t;

typedef struct  rtk_int_status_s
{
    rtk_uint8 value[RTK_MAX_NUM_OF_INTERRUPT_TYPE];
} rtk_int_status_t;

typedef rtk_uint32 rtk_ipaddr_t;

typedef rtk_uint32 rtk_l2_age_time_t;

typedef enum rtk_l2_flood_type_e
{
    FLOOD_UNKNOWNDA = 0,
    FLOOD_UNKNOWNMC,
    FLOOD_BC,
    FLOOD_END
} rtk_l2_flood_type_t;

typedef rtk_uint32 rtk_l2_flushItem_t;

typedef rtk_uint32  rtk_vlan_t;        /* vlan id type */
typedef struct  rtk_vlan_mbrcfg_s
{
    rtk_uint16 	evid;
	rtk_uint16 	mbr;
    rtk_uint16  fid_msti;
    rtk_uint16  envlanpol;
    rtk_uint16  meteridx;
    rtk_uint16  vbpen;
    rtk_uint16  vbpri;
}rtk_vlan_mbrcfg_t;

typedef rtk_uint32  rtk_port_t;        /* port is type */

typedef enum rtk_l2_flushType_e
{
    FLUSH_TYPE_BY_PORT = 0,       /* physical port       */
    FLUSH_TYPE_BY_PORT_VID,       /* physical port + VID */
    FLUSH_TYPE_END
} rtk_l2_flushType_t;

typedef struct rtk_l2_flushCfg_s
{
    rtk_enable_t    flushByVid;
    rtk_vlan_t      vid;
    rtk_enable_t    flushByPort;
    rtk_enable_t    reserved;
    rtk_port_t      port;
    rtk_enable_t    flushByMac;
    rtk_mac_t       ucastAddr;
    rtk_enable_t    flushStaticAddr;
    rtk_enable_t    flushAddrOnAllPorts; /* this is used when flushByVid */
} rtk_l2_flushCfg_t;

typedef enum rtk_l2_hash_method_e
{
    HSAH_NORMAL = 0,
    HASH_IGNORE_SIP,
    HASH_END
} rtk_hash_method_t;

typedef enum rtk_l2_read_method_e{

	READMETHOD_MAC = 0,
	READMETHOD_ADDRESS,
	READMETHOD_NEXT_ADDRESS,
	READMETHOD_NEXT_L2UC,
	READMETHOD_NEXT_L2MC,
	READMETHOD_NEXT_L3MC,
	READMETHOD_NEXT_L2L3MC,
	READMETHOD_NEXT_L2UCSPA,
    READMETHOD_END
}rtk_l2_read_method_t;

/* l2 limit learning count action */
typedef enum rtk_l2_limitLearnCntAction_e
{
    LIMIT_LEARN_CNT_ACTION_DROP = 0,
    LIMIT_LEARN_CNT_ACTION_FORWARD,
    LIMIT_LEARN_CNT_ACTION_TO_CPU,
    LIMIT_LEARN_CNT_ACTION_END
} rtk_l2_limitLearnCntAction_t;

typedef enum rtk_l2_lookup_type_e
{
    LOOKUP_MAC = 0,
    LOOKUP_SIP_DIP,
    LOOKUP_DIP,
    LOOKUP_END
} rtk_l2_lookup_type_t;

/* l2 address table - unicast data structure */
typedef struct rtk_l2_ucastAddr_s
{
    rtk_mac_t       mac;
    rtk_uint32      ivl;
    rtk_uint32      cvid;
    rtk_uint32      fid;
    rtk_uint32      efid;
    rtk_uint32      port;
    rtk_uint32      sa_block;
    rtk_uint32      da_block;
    rtk_uint32      auth;
    rtk_uint32      is_static;
}rtk_l2_ucastAddr_t;


typedef enum rtk_leaky_type_e
{
    LEAKY_BRG_GROUP = 0,
    LEAKY_FD_PAUSE,
    LEAKY_SP_MCAST,
    LEAKY_1X_PAE,
    LEAKY_UNDEF_BRG_04,
    LEAKY_UNDEF_BRG_05,
    LEAKY_UNDEF_BRG_06,
    LEAKY_UNDEF_BRG_07,
    LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
    LEAKY_UNDEF_BRG_09,
    LEAKY_UNDEF_BRG_0A,
    LEAKY_UNDEF_BRG_0B,
    LEAKY_UNDEF_BRG_0C,
    LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
    LEAKY_8021AB,
    LEAKY_UNDEF_BRG_0F,
    LEAKY_BRG_MNGEMENT,
    LEAKY_UNDEFINED_11,
    LEAKY_UNDEFINED_12,
    LEAKY_UNDEFINED_13,
    LEAKY_UNDEFINED_14,
    LEAKY_UNDEFINED_15,
    LEAKY_UNDEFINED_16,
    LEAKY_UNDEFINED_17,
    LEAKY_UNDEFINED_18,
    LEAKY_UNDEFINED_19,
    LEAKY_UNDEFINED_1A,
    LEAKY_UNDEFINED_1B,
    LEAKY_UNDEFINED_1C,
    LEAKY_UNDEFINED_1D,
    LEAKY_UNDEFINED_1E,
    LEAKY_UNDEFINED_1F,
    LEAKY_GMRP,
    LEAKY_GVRP,
    LEAKY_UNDEF_GARP_22,
    LEAKY_UNDEF_GARP_23,
    LEAKY_UNDEF_GARP_24,
    LEAKY_UNDEF_GARP_25,
    LEAKY_UNDEF_GARP_26,
    LEAKY_UNDEF_GARP_27,
    LEAKY_UNDEF_GARP_28,
    LEAKY_UNDEF_GARP_29,
    LEAKY_UNDEF_GARP_2A,
    LEAKY_UNDEF_GARP_2B,
    LEAKY_UNDEF_GARP_2C,
    LEAKY_UNDEF_GARP_2D,
    LEAKY_UNDEF_GARP_2E,
    LEAKY_UNDEF_GARP_2F,
    LEAKY_IGMP,
    LEAKY_IPMULTICAST,
    LEAKY_END,
}rtk_leaky_type_t;


typedef enum rtk_led_operation_e
{
    LED_OP_SCAN=0,
    LED_OP_PARALLEL,
    LED_OP_SERIAL,
    LED_OP_END,
}rtk_led_operation_t;


typedef enum rtk_led_active_e
{
    LED_ACTIVE_HIGH=0,
    LED_ACTIVE_LOW,
    LED_ACTIVE_END,
}rtk_led_active_t;

typedef enum rtk_led_config_e
{
    LED_CONFIG_LEDOFF=0,
    LED_CONFIG_DUPCOL,
    LED_CONFIG_LINK_ACT,
    LED_CONFIG_SPD1000,
    LED_CONFIG_SPD100,
    LED_CONFIG_SPD10,
    LED_CONFIG_SPD1000ACT,
    LED_CONFIG_SPD100ACT,
    LED_CONFIG_SPD10ACT,
    LED_CONFIG_SPD10010ACT,
    LED_CONFIG_LOOPDETECT,
    LED_CONFIG_EEE,
    LED_CONFIG_LINKRX,
    LED_CONFIG_LINKTX,
    LED_CONFIG_MASTER,
    LED_CONFIG_ACT,
    LED_CONFIG_END,
}rtk_led_congig_t;

typedef enum rtk_led_blink_rate_e
{
    LED_BLINKRATE_32MS=0,
    LED_BLINKRATE_64MS,
    LED_BLINKRATE_128MS,
    LED_BLINKRATE_256MS,
    LED_BLINKRATE_512MS,
    LED_BLINKRATE_1024MS,
    LED_BLINKRATE_48MS,
    LED_BLINKRATE_96MS,
    LED_BLINKRATE_END,
}rtk_led_blink_rate_t;

typedef enum rtk_led_group_e
{
    LED_GROUP_0 = 0,
    LED_GROUP_1,
    LED_GROUP_2,
    LED_GROUP_END
}rtk_led_group_t;

typedef enum rtk_led_mode_e
{
    LED_MODE_0 = 0,
    LED_MODE_1,
    LED_MODE_2,
    LED_MODE_3,
    LED_MODE_END
}rtk_led_mode_t;


typedef enum rtk_led_force_mode_e
{
    LED_FORCE_NORMAL=0,
    LED_FORCE_BLINK,
    LED_FORCE_OFF,
    LED_FORCE_ON,
    LED_FORCE_END
}rtk_led_force_mode_t;


typedef struct rtk_l2_addr_table_s
{
	rtk_int32 	index;
	ipaddr_t    sip;
	ipaddr_t    dip;
	rtk_mac_t   mac;
	rtk_uint32 	sa_block;
	rtk_uint32 	auth;
	rtk_uint32 	portmask;
	rtk_uint32 	age;
    rtk_uint32 	ivl;
    rtk_uint32 	cvid;
	rtk_uint32 	fid;
	rtk_uint32 	is_ipmul;
	rtk_uint32 	is_static;
}rtk_l2_addr_table_t;


typedef rtk_uint32  rtk_mac_cnt_t;     /* meter id type  */

typedef enum rtk_mcast_type_e
{
    MCAST_L2 = 0,
    MCAST_IPV4,
    MCAST_IPV6,
    MCAST_END
} rtk_mcast_type_t;

typedef rtk_uint32  rtk_meter_id_t;     /* meter id type  */

typedef enum rtk_mode_e
{
    MODE0 = 0,
    MODE1,
    MODE_END
} rtk_mode_t;

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef struct  rtk_port_mac_ability_s
{
    rtk_uint32 forcemode;
    rtk_uint32 speed;
    rtk_uint32 duplex;
    rtk_uint32 link;
    rtk_uint32 nway;
    rtk_uint32 txpause;
    rtk_uint32 rxpause;
}rtk_port_mac_ability_t;

typedef struct rtk_port_phy_ability_s
{
    rtk_uint32    AutoNegotiation;  /*PHY register 0.12 setting for auto-negotiation process*/
    rtk_uint32    Half_10;          /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    rtk_uint32    Full_10;          /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    rtk_uint32    Half_100;         /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    rtk_uint32    Full_100;         /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    rtk_uint32    Full_1000;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    rtk_uint32    FC;               /*PHY register 4.10 setting for flow control capability*/
    rtk_uint32    AsyFC;            /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtk_port_phy_ability_t;

typedef rtk_uint32  rtk_port_phy_data_t;     /* phy page  */

typedef enum rtk_port_phy_mdix_mode_e
{
    PHY_AUTO_CROSSOVER_MODE= 0,
    PHY_FORCE_MDI_MODE,
    PHY_FORCE_MDIX_MODE,
    PHY_FORCE_MODE_END
} rtk_port_phy_mdix_mode_t;

typedef enum rtk_port_phy_mdix_status_e
{
    PHY_STATUS_AUTO_MDI_MODE= 0,
    PHY_STATUS_AUTO_MDIX_MODE,
    PHY_STATUS_FORCE_MDI_MODE,
    PHY_STATUS_FORCE_MDIX_MODE,
    PHY_STATUS_FORCE_MODE_END
} rtk_port_phy_mdix_status_t;

typedef rtk_uint32  rtk_port_phy_page_t;     /* phy page  */

typedef enum rtk_port_phy_reg_e
{
    PHY_REG_CONTROL             = 0,
    PHY_REG_STATUS,
    PHY_REG_IDENTIFIER_1,
    PHY_REG_IDENTIFIER_2,
    PHY_REG_AN_ADVERTISEMENT,
    PHY_REG_AN_LINKPARTNER,
    PHY_REG_1000_BASET_CONTROL  = 9,
    PHY_REG_1000_BASET_STATUS,
    PHY_REG_END                 = 32
} rtk_port_phy_reg_t;

typedef enum rtk_port_phy_test_mode_e
{
    PHY_TEST_MODE_NORMAL= 0,
    PHY_TEST_MODE_1,
    PHY_TEST_MODE_2,
    PHY_TEST_MODE_3,
    PHY_TEST_MODE_4,
    PHY_TEST_MODE_END
} rtk_port_phy_test_mode_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef enum rtk_port_media_e
{
    PORT_MEDIA_COPPER = 0,
    PORT_MEDIA_FIBER,
    PORT_MEDIA_END
}rtk_port_media_t;

typedef struct rtk_portmask_s
{
    rtk_uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef struct rtk_rtctResult_s
{
    rtk_port_speed_t    linkType;
    union
    {
        struct fe_result_s
        {
            rtk_uint32      isRxShort;
            rtk_uint32      isTxShort;
            rtk_uint32      isRxOpen;
            rtk_uint32      isTxOpen;
            rtk_uint32      isRxMismatch;
            rtk_uint32      isTxMismatch;
            rtk_uint32      isRxLinedriver;
            rtk_uint32      isTxLinedriver;
            rtk_uint32      rxLen;
            rtk_uint32      txLen;
        } fe_result;

        struct ge_result_s
        {
            rtk_uint32      channelAShort;
            rtk_uint32      channelBShort;
            rtk_uint32      channelCShort;
            rtk_uint32      channelDShort;

            rtk_uint32      channelAOpen;
            rtk_uint32      channelBOpen;
            rtk_uint32      channelCOpen;
            rtk_uint32      channelDOpen;

            rtk_uint32      channelAMismatch;
            rtk_uint32      channelBMismatch;
            rtk_uint32      channelCMismatch;
            rtk_uint32      channelDMismatch;

            rtk_uint32      channelALinedriver;
            rtk_uint32      channelBLinedriver;
            rtk_uint32      channelCLinedriver;
            rtk_uint32      channelDLinedriver;

            rtk_uint32      channelALen;
            rtk_uint32      channelBLen;
            rtk_uint32      channelCLen;
            rtk_uint32      channelDLen;
        } ge_result;
    }result;
} rtk_rtctResult_t;

typedef rtk_uint32  rtk_pri_t;         /* priority vlaue */

typedef struct rtk_priority_select_s
{
    rtk_uint32 port_pri;
    rtk_uint32 dot1q_pri;
    rtk_uint32 acl_pri;
    rtk_uint32 dscp_pri;
    rtk_uint32 cvlan_pri;
    rtk_uint32 svlan_pri;
    rtk_uint32 dmac_pri;
    rtk_uint32 smac_pri;
} rtk_priority_select_t;


typedef rtk_uint32  rtk_qid_t;        /* queue id type */

typedef struct rtk_qos_pri2queue_s
{
    rtk_uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
} rtk_qos_pri2queue_t;

typedef struct rtk_qos_queue_weights_s
{
    rtk_uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef enum rtk_qos_scheduling_type_e
{
    WFQ = 0,        /* Weighted-Fair-Queue */
    WRR,            /* Weighted-Round-Robin */
    SCHEDULING_TYPE_END
} rtk_qos_scheduling_type_t;

typedef rtk_uint32  rtk_queue_num_t;    /* queue number*/

typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_END
} rtk_rate_storm_group_t;

typedef rtk_uint32  rtk_rate_t;     /* rate type  */

typedef rtk_u_long_t rtk_stat_counter_t;

#ifndef EMBEDDED_SUPPORT
/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    rtk_uint64 dot1dTpLearnedEntryDiscards;
}rtk_stat_global_cntr_t;
#endif

typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 58,
    MIB_GLOBAL_CNTR_END
}rtk_stat_global_type_t;

#ifndef EMBEDDED_SUPPORT
/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    rtk_uint64 ifInOctets;
    rtk_uint32 dot3StatsFCSErrors;
    rtk_uint32 dot3StatsSymbolErrors;
    rtk_uint32 dot3InPauseFrames;
    rtk_uint32 dot3ControlInUnknownOpcodes;
    rtk_uint32 etherStatsFragments;
    rtk_uint32 etherStatsJabbers;
    rtk_uint32 ifInUcastPkts;
    rtk_uint32 etherStatsDropEvents;
    rtk_uint64 etherStatsOctets;
    rtk_uint32 etherStatsUndersizePkts;
    rtk_uint32 etherStatsOversizePkts;
    rtk_uint32 etherStatsPkts64Octets;
    rtk_uint32 etherStatsPkts65to127Octets;
    rtk_uint32 etherStatsPkts128to255Octets;
    rtk_uint32 etherStatsPkts256to511Octets;
    rtk_uint32 etherStatsPkts512to1023Octets;
    rtk_uint32 etherStatsPkts1024toMaxOctets;
    rtk_uint32 etherStatsMcastPkts;
    rtk_uint32 etherStatsBcastPkts;
    rtk_uint64 ifOutOctets;
    rtk_uint32 dot3StatsSingleCollisionFrames;
    rtk_uint32 dot3StatsMultipleCollisionFrames;
    rtk_uint32 dot3StatsDeferredTransmissions;
    rtk_uint32 dot3StatsLateCollisions;
    rtk_uint32 etherStatsCollisions;
    rtk_uint32 dot3StatsExcessiveCollisions;
    rtk_uint32 dot3OutPauseFrames;
    rtk_uint32 dot1dBasePortDelayExceededDiscards;
    rtk_uint32 dot1dTpPortInDiscards;
    rtk_uint32 ifOutUcastPkts;
    rtk_uint32 ifOutMulticastPkts;
    rtk_uint32 ifOutBrocastPkts;
    rtk_uint32 outOampduPkts;
    rtk_uint32 inOampduPkts;
    rtk_uint32 pktgenPkts;
#if 0
    /* reduce MIB counters, align to RTL8370 API */
    rtk_uint32 inMldChecksumError;
    rtk_uint32 inIgmpChecksumError;
    rtk_uint32 inMldSpecificQuery;
    rtk_uint32 inMldGeneralQuery;
    rtk_uint32 inIgmpSpecificQuery;
    rtk_uint32 inIgmpGeneralQuery;
    rtk_uint32 inMldLeaves;
    rtk_uint32 inIgmpLeaves;
    rtk_uint32 inIgmpJoinsSuccess;
    rtk_uint32 inIgmpJoinsFail;
    rtk_uint32 inMldJoinsSuccess;
    rtk_uint32 inMldJoinsFail;
    rtk_uint32 inReportSuppressionDrop;
    rtk_uint32 inLeaveSuppressionDrop;
    rtk_uint32 outIgmpReports;
    rtk_uint32 outIgmpLeaves;
    rtk_uint32 outIgmpGeneralQuery;
    rtk_uint32 outIgmpSpecificQuery;
    rtk_uint32 outMldReports;
    rtk_uint32 outMldLeaves;
    rtk_uint32 outMldGeneralQuery;
    rtk_uint32 outMldSpecificQuery;
    rtk_uint32 inKnownMulticastPkts;
    rtk_uint32 ifInMulticastPkts;
    rtk_uint32 ifInBroadcastPkts;
#endif
}rtk_stat_port_cntr_t;
#endif

/* port statistic counter index */
typedef enum rtk_stat_port_type_e
{
    STAT_IfInOctets = 0,
    STAT_Dot3StatsFCSErrors,
    STAT_Dot3StatsSymbolErrors,
    STAT_Dot3InPauseFrames,
    STAT_Dot3ControlInUnknownOpcodes,
    STAT_EtherStatsFragments,
    STAT_EtherStatsJabbers,
    STAT_IfInUcastPkts,
    STAT_EtherStatsDropEvents,
    STAT_EtherStatsOctets,
    STAT_EtherStatsUnderSizePkts,
    STAT_EtherOversizeStats,
    STAT_EtherStatsPkts64Octets,
    STAT_EtherStatsPkts65to127Octets,
    STAT_EtherStatsPkts128to255Octets,
    STAT_EtherStatsPkts256to511Octets,
    STAT_EtherStatsPkts512to1023Octets,
    STAT_EtherStatsPkts1024to1518Octets,
    STAT_EtherStatsMulticastPkts,
    STAT_EtherStatsBroadcastPkts,
    STAT_IfOutOctets,
    STAT_Dot3StatsSingleCollisionFrames,
    STAT_Dot3StatsMultipleCollisionFrames,
    STAT_Dot3StatsDeferredTransmissions,
    STAT_Dot3StatsLateCollisions,
    STAT_EtherStatsCollisions,
    STAT_Dot3StatsExcessiveCollisions,
    STAT_Dot3OutPauseFrames,
    STAT_Dot1dBasePortDelayExceededDiscards,
    STAT_Dot1dTpPortInDiscards,
    STAT_IfOutUcastPkts,
    STAT_IfOutMulticastPkts,
    STAT_IfOutBroadcastPkts,
    STAT_OutOampduPkts,
    STAT_InOampduPkts,
    STAT_PktgenPkts,
#if 0
    /* reduce MIB counters, align to RTL8370 API */
    STAT_InMldChecksumError,
    STAT_InIgmpChecksumError,
    STAT_InMldSpecificQuery,
    STAT_InMldGeneralQuery,
    STAT_InIgmpSpecificQuery,
    STAT_InIgmpGeneralQuery,
    STAT_InMldLeaves,
    STAT_InIgmpInterfaceLeaves,
    STAT_InIgmpJoinsSuccess,
    STAT_InIgmpJoinsFail,
    STAT_InMldJoinsSuccess,
    STAT_InMldJoinsFail,
    STAT_InReportSuppressionDrop,
    STAT_InLeaveSuppressionDrop,
    STAT_OutIgmpReports,
    STAT_OutIgmpLeaves,
    STAT_OutIgmpGeneralQuery,
    STAT_OutIgmpSpecificQuery,
    STAT_OutMldReports,
    STAT_OutMldLeaves,
    STAT_OutMldGeneralQuery,
    STAT_OutMldSpecificQuery,
    STAT_InKnownMulticastPkts,
    STAT_IfInMulticastPkts,
    STAT_IfInBroadcastPkts,
#endif
    STAT_PORT_CNTR_END
}rtk_stat_port_type_t;

typedef enum rtk_logging_counter_mode_e
{
    LOGGING_MODE_32BIT = 0,
    LOGGING_MODE_64BIT,
    LOGGING_MODE_END
}rtk_logging_counter_mode_t;

typedef enum rtk_logging_counter_type_e
{
    LOGGING_TYPE_PACKET = 0,
    LOGGING_TYPE_BYTE,
    LOGGING_TYPE_END
}rtk_logging_counter_type_t;

typedef rtk_uint32  rtk_stg_t;        /* spanning tree instance id type */

typedef enum rtk_storm_bypass_e
{
    BYPASS_BRG_GROUP = 0,
    BYPASS_FD_PAUSE,
    BYPASS_SP_MCAST,
    BYPASS_1X_PAE,
    BYPASS_UNDEF_BRG_04,
    BYPASS_UNDEF_BRG_05,
    BYPASS_UNDEF_BRG_06,
    BYPASS_UNDEF_BRG_07,
    BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
    BYPASS_UNDEF_BRG_09,
    BYPASS_UNDEF_BRG_0A,
    BYPASS_UNDEF_BRG_0B,
    BYPASS_UNDEF_BRG_0C,
    BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
    BYPASS_8021AB,
    BYPASS_UNDEF_BRG_0F,
    BYPASS_BRG_MNGEMENT,
    BYPASS_UNDEFINED_11,
    BYPASS_UNDEFINED_12,
    BYPASS_UNDEFINED_13,
    BYPASS_UNDEFINED_14,
    BYPASS_UNDEFINED_15,
    BYPASS_UNDEFINED_16,
    BYPASS_UNDEFINED_17,
    BYPASS_UNDEFINED_18,
    BYPASS_UNDEFINED_19,
    BYPASS_UNDEFINED_1A,
    BYPASS_UNDEFINED_1B,
    BYPASS_UNDEFINED_1C,
    BYPASS_UNDEFINED_1D,
    BYPASS_UNDEFINED_1E,
    BYPASS_UNDEFINED_1F,
    BYPASS_GMRP,
    BYPASS_GVRP,
    BYPASS_UNDEF_GARP_22,
    BYPASS_UNDEF_GARP_23,
    BYPASS_UNDEF_GARP_24,
    BYPASS_UNDEF_GARP_25,
    BYPASS_UNDEF_GARP_26,
    BYPASS_UNDEF_GARP_27,
    BYPASS_UNDEF_GARP_28,
    BYPASS_UNDEF_GARP_29,
    BYPASS_UNDEF_GARP_2A,
    BYPASS_UNDEF_GARP_2B,
    BYPASS_UNDEF_GARP_2C,
    BYPASS_UNDEF_GARP_2D,
    BYPASS_UNDEF_GARP_2E,
    BYPASS_UNDEF_GARP_2F,
    BYPASS_IGMP,
    BYPASS_END,
}rtk_storm_bypass_t;

typedef rtk_uint32  rtk_stp_msti_id_t;     /* MSTI ID  */

typedef enum rtk_stp_state_e
{
    STP_STATE_DISABLED = 0,
    STP_STATE_BLOCKING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_END
} rtk_stp_state_t;

typedef rtk_uint32 rtk_svlan_index_t;

typedef struct rtk_svlan_memberCfg_s{
    rtk_uint32 svid;
    rtk_uint32 memberport;
    rtk_uint32 untagport;
    rtk_uint32 fiden;
    rtk_uint32 fid;
    rtk_uint32 priority;
    rtk_uint32 efiden;
    rtk_uint32 efid;
}rtk_svlan_memberCfg_t;

typedef enum rtk_svlan_pri_ref_e
{
    REF_INTERNAL_PRI = 0,
    REF_CTAG_PRI,
    REF_SVLAN_PRI,
    REF_PRI_END
} rtk_svlan_pri_ref_t;


typedef rtk_uint32 rtk_svlan_tpid_t;

typedef enum rtk_svlan_untag_action_e
{
    UNTAG_DROP = 0,
    UNTAG_TRAP,
    UNTAG_ASSIGN,
    UNTAG_END
} rtk_svlan_untag_action_t;

typedef enum rtk_svlan_unmatch_action_e
{
    UNMATCH_DROP = 0,
    UNMATCH_TRAP,
    UNMATCH_ASSIGN,
    UNMATCH_END
} rtk_svlan_unmatch_action_t;

typedef enum rtk_switch_maxPktLen_e
{
    MAXPKTLEN_1522B = 0,
    MAXPKTLEN_1536B,
    MAXPKTLEN_1552B,
    MAXPKTLEN_16000B,
    MAXPKTLEN_END
} rtk_switch_maxPktLen_t;

typedef enum rtk_trap_igmp_action_e
{
    IGMP_ACTION_FORWARD = 0,
    IGMP_ACTION_TRAP2CPU,
    IGMP_ACTION_DROP,
    IGMP_ACTION_FORWARD_EXCLUDE_CPU,
    IGMP_ACTION_ASIC,
    IGMP_ACTION_END
} rtk_trap_igmp_action_t;

typedef enum rtk_trap_mcast_action_e
{
    MCAST_ACTION_FORWARD = 0,
    MCAST_ACTION_DROP,
    MCAST_ACTION_TRAP2CPU,
    MCAST_ACTION_ROUTER_PORT,
    MCAST_ACTION_END
} rtk_trap_mcast_action_t;

typedef enum rtk_igmp_protocol_e
{
    PROTOCOL_IGMPv1 = 0,
    PROTOCOL_IGMPv2,
    PROTOCOL_IGMPv3,
    PROTOCOL_MLDv1,
    PROTOCOL_MLDv2,
    PROTOCOL_END
} rtk_igmp_protocol_t;

typedef enum rtk_trap_reason_type_e
{
    TRAP_REASON_RMA = 0,
    TRAP_REASON_IPV4IGMP,
    TRAP_REASON_IPV6MLD,
    TRAP_REASON_1XEAPOL,
    TRAP_REASON_VLANERR,
    TRAP_REASON_SLPCHANGE,
    TRAP_REASON_MULTICASTDLF,
    TRAP_REASON_CFI,
    TRAP_REASON_1XUNAUTH,
    TRAP_REASON_END,
} rtk_trap_reason_type_t;


typedef enum rtk_trap_rma_action_e
{
    RMA_ACTION_FORWARD = 0,
    RMA_ACTION_TRAP2CPU,
    RMA_ACTION_DROP,
    RMA_ACTION_FORWARD_EXCLUDE_CPU,
    RMA_ACTION_END
} rtk_trap_rma_action_t;

typedef enum rtk_trap_ucast_action_e
{
    UCAST_ACTION_FORWARD = 0,
    UCAST_ACTION_DROP,
    UCAST_ACTION_TRAP2CPU,
    UCAST_ACTION_END
} rtk_trap_ucast_action_t;

typedef enum rtk_trap_ucast_type_e
{
    UCAST_UNKNOWNDA = 0,
    UCAST_UNKNOWNSA,
    UCAST_UNMATCHSA,
    UCAST_END
} rtk_trap_ucast_type_t;

typedef enum rtk_trunk_group_e
{
    TRUNK_GROUP0 = 0,
    TRUNK_GROUP1,
    TRUNK_GROUP2,
    TRUNK_GROUP3,
    TRUNK_GROUP_END
} rtk_trunk_group_t;

typedef struct  rtk_trunk_hashVal2Port_s
{
    rtk_uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef rtk_uint32  rtk_vlan_proto_type_t;     /* protocol and port based VLAN protocol type  */


typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,             /* untagged, priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,     /* untagged and priority-tagged */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;


/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_RFC1042,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    rtk_uint32                         proto_type;
    rtk_vlan_protoVlan_frameType_t frame_type;
    rtk_vlan_t                     cvid;
    rtk_pri_t                     cpri;
}rtk_vlan_protoAndPortInfo_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,
    VLAN_TAG_MODE_KEEP_FORMAT,
    VLAN_TAG_MODE_PRI,
    VLAN_TAG_MODE_REAL_KEEP_FORMAT,
    VLAN_TAG_MODE_END
} rtk_vlan_tagMode_t;

typedef enum rtk_ext_port_e
{
    EXT_PORT_0 = 0,
    EXT_PORT_1,
    EXT_PORT_2,
    EXT_PORT_END
}rtk_ext_port_t;

typedef enum rtk_rldp_transmitMode_e
{
    RLDP_TRANSMIT_MODE_0 = 0,
    RLDP_TRANSMIT_MODE_1,
    RLDP_TRANSMIT_MODE_END
} rtk_rldp_transmitMode_t;


#endif /* __RTK_API_H__ */

