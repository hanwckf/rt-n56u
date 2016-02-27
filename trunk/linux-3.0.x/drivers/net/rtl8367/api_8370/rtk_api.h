/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:45 $
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
#define PHY_POWERSAVING_REG                         21
#define PHY_POWERSAVING_OFFSET                      12
#define PHY_POWERSAVING_MASK                        0x1000
#define	PHY_PAGE_ADDRESS                            31

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

#define ACL_DEFAULT_ABILITY                         0
#define ACL_DEFAULT_UNMATCH_PERMIT                  1

#define ACL_RULE_FREE                               0
#define ACL_RULE_INAVAILABLE                        1

#define FILTER_POLICING_MAX                         8
#define FILTER_LOGGING_MAX                          8
#define FILTER_PATTERN_MAX                          4

#define STORM_UNUC_INDEX                            39
#define STORM_UNMC_INDEX                            47
#define STORM_MC_INDEX                              55
#define STORM_BC_INDEX                              63

#define RTK_MAX_NUM_OF_INTERRUPT_TYPE               1
#define RTK_MAX_NUM_OF_LED_GROUP                    3
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1
#define RTK_MAX_NUM_OF_PRIORITY                     8
#define RTK_MAX_NUM_OF_QUEUE                        8
#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               1
#define RTK_MAX_NUM_OF_INTERRUPT_TYPE               1
#define RTK_MAX_NUM_OF_PORT                         10
#define RTK_PORT_ID_MAX                             (RTK_MAX_NUM_OF_PORT-1)
#define RTK_PHY_ID_MAX                             (RTK_MAX_NUM_OF_PORT-3)
#define RTK_QUEUE_ID_MAX                           (RTK_MAX_NUM_OF_QUEUE-1)  
#define RTK_MAX_NUM_OF_PROTO_TYPE                   0xFFFF
#define RTK_MAX_NUM_OF_MSTI                         16
#define RTK_MAX_NUM_OF_LEARN_LIMIT                  0x2040
#define RTK_MAX_PORT_MASK                           0x3FF
#define RTK_MAX_INPUT_RATE                         (0x1FFFF * 8)
#define RTK_MIN_INPUT_RATE                          8
#define RTK_RATE_GRANULARTY_UNIT                    8
#define RTK_DOT1X_PAE                               3
#define RTK_L2_DEFAULT_TIME                         6
#define RTK_L2_DEFAULT_SPEED                        2
#define RTK_MAX_NUM_OF_SVLAN_INDEX                  64
#define RTK_MAX_NUM_OF_SP2C_INDEX                  128
#define RTK_MAX_NUM_OF_MC2S_INDEX                   32
#define RTK_MAX_NUM_OF_C2S_INDEX                   128
#define RTK_VLAN_ID_MIN                             0
#define RTK_VLAN_ID_MAX                             4095
#define RTK_DOT1P_PRIORITY_MAX                      7
#define RTK_MAX_NUM_OF_FILTER_TYPE                  5
#define RTK_MAX_NUM_OF_FILTER_FIELD                 7
#define RTK_MAX_NUM_OF_FILTER_PORT                  16
#define RTK_MAX_NUM_OF_METER                        64
#define RTK_QOS_RATE_INPUT_MAX                     (0x1FFFF * 8)
#define RTK_QOS_RATE_INPUT_MIN                      8
#define RTK_VALUE_OF_DSCP_MAX                       63
#define RTK_VALUE_OF_DSCP_MIN                       0
#define RTK_EFID_MAX                                0x7
#define RTK_FID_MAX                                 0xFFF
#define RTK_MAX_NUM_OF_VLAN_INDEX                   32
#define RTK_MAX_NUM_OF_PROTOVLAN_GROUP              4
#define RTK_PROTOVLAN_GROUP_ID_MAX                  (RTK_MAX_NUM_OF_PROTOVLAN_GROUP-1)
#define RTK_MAX_NUM_OF_ACL_RULE                     64
#define RTK_SVLAN_TPID                              0x88a8
#define RTK_PORT_TRUNK_GROUP_MASK(group)           (0xF<<(group<<2))
#define RTK_PORT_TRUNK_GROUP_OFFSET(group)         (group << 2)


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


#define RTK_WHOLE_SYSTEM                            0xFF

#define RTK_EXT_0                                   0
#define RTK_EXT_1                                   1

#define RTK_EXT_0_MAC9                              9
#define RTK_EXT_1_MAC8                              8

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN                                6
#endif

#define IPV6_ADDR_LEN                               16
#define IPV4_ADDR_LEN                               4

#define RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH   3UL

#define RTK_IPV6_ADDR_WORD_LENGTH                   4UL


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

typedef uint32  rtk_data_t; 

/* Type of port-based dot1x auth/unauth*/
typedef enum rtk_dot1x_auth_status_e
{
    UNAUTH = 0,
    AUTH,
    AUTH_STATUS_END
} rtk_dot1x_auth_status_t;

typedef enum rtk_dot1x_direction_e
{
    BOTH = 0,
    IN,
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
    MODE_EXT_RGMII_33V,   
    MODE_EXT_END
} rtk_mode_ext_t;

typedef struct
{
    uint32 value[RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH];
} rtk_filter_dot1as_timestamp_t;

/* unauth pkt action */
typedef enum rtk_dot1x_unauth_action_e
{
    DOT1X_ACTION_DROP = 0,
    DOT1X_ACTION_TRAP2CPU,
    DOT1X_ACTION_GUESTVLAN,
    DOT1X_ACTION_END
} rtk_dot1x_unauth_action_t;

typedef uint32  rtk_dscp_t;         /* dscp vlaue */

typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef uint32  rtk_fid_t;        /* filter id type */

/* ethernet address type */
typedef struct  rtk_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;

typedef enum rtk_filter_act_enable_e
{
    /* CVLAN */
    FILTER_ENACT_INGRESS_CVLAN_INDEX = 0,
    FILTER_ENACT_INGRESS_CVLAN_VID,

    /* SVLAN */
    FILTER_ENACT_EGRESS_SVLAN_INDEX,

    /* Policing and Logging */
    FILTER_ENACT_POLICING_0,   

    /* Forward */
    FILTER_ENACT_TRAP_CPU,
    FILTER_ENACT_COPY_CPU,
    FILTER_ENACT_REDIRECT,
    FILTER_ENACT_DROP,    
    FILTER_ENACT_MIRROR,    
    FILTER_ENACT_ADD_DSTPORT,    

    /* QoS */
    FILTER_ENACT_PRIORITY,

    /* SVLAN */
    FILTER_ENACT_EGRESS_SVLAN_VID,

    FILTER_ENACT_MAX,
} rtk_filter_act_enable_t;

typedef struct
{    	
    rtk_filter_act_enable_t actEnable[FILTER_ENACT_MAX];

	 /* CVLAN acton */
	 uint32        filterIngressCvlanIdx;
	 uint32        filterIngressCvlanVid;
	 	
	 /* SVLAN action */
	 uint32        filterEgressSvlanIdx;	 
	 uint32        filterEgressSvlanVid;
	 
	 /* Policing action */
	 uint32        filterPolicingIdx[FILTER_POLICING_MAX];

	 /* Forwarding action */
    uint32        filterRedirectPortmask;
    uint32        filterAddDstPortmask;  
	
	/* QOS action */
    uint32        filterPriority;
    
} rtk_filter_action_t;

typedef enum rtk_filter_fwd_act_e
{
    RTK_FILTER_FWD_MIRROR = 0,
    RTK_FILTER_FWD_REDIRECT,
    RTK_FILTER_FWD_MIRRORFUNTION,
    RTK_FILTER_FWD_TRAP,
} rtk_filter_fwd_act_t;


typedef struct
{
    uint32 value;
    uint32 mask;
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
    CARE_TAG_ICMP,    
    CARE_TAG_IGMP,    
    CARE_TAG_MAX,        
} rtk_filter_care_tag_index_t;

typedef struct
{
    rtk_filter_flag_t tagType[CARE_TAG_MAX];
} rtk_filter_care_tag_t;

typedef struct rtk_filter_field rtk_filter_field_t;

typedef enum rtk_filter_field_data_type_e
{
    FILTER_FIELD_DATA_MASK = 0,
    FILTER_FIELD_DATA_RANGE,        
    FILTER_FIELD_DATA_MAX ,
} rtk_filter_field_data_type;

typedef struct
{
    uint32 dataType;
    uint32 rangeStart;
    uint32 rangeEnd;		
    uint32 value;
    uint32 mask;
} rtk_filter_ip_t;

typedef struct
{
    uint32 dataType;
    rtk_mac_t value;
    rtk_mac_t mask;
    rtk_mac_t rangeStart;
    rtk_mac_t rangeEnd;
} rtk_filter_mac_t;

typedef uint32 rtk_filter_op_t;

typedef struct rtk_filter_value_s
{
    uint32 dataType;
    uint32 value;
    uint32 mask;	
    uint32 rangeStart;
    uint32 rangeEnd;
} rtk_filter_value_t;

typedef struct rtk_filter_tag_s
{
    rtk_filter_value_t pri;
    rtk_filter_flag_t cfi;
    rtk_filter_value_t vid;
} rtk_filter_tag_t;

typedef struct
{
    rtk_filter_flag_t mf;
    rtk_filter_flag_t df;
} rtk_filter_ipFlag_t;

typedef struct
{
    uint32 addr[RTK_IPV6_ADDR_WORD_LENGTH];
} rtk_filter_ip6_addr_t;

typedef struct
{
    uint32 dataType;
    rtk_filter_ip6_addr_t value;
    rtk_filter_ip6_addr_t mask;	
    rtk_filter_ip6_addr_t rangeStart;
    rtk_filter_ip6_addr_t rangeEnd;
} rtk_filter_ip6_t;

typedef uint32 rtk_filter_number_t;

typedef struct
{
    uint32 value[FILTER_PATTERN_MAX];
    uint32 mask[FILTER_PATTERN_MAX];
} rtk_filter_pattern_t;

typedef struct
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

typedef uint32 rtk_filter_field_raw_t;

struct rtk_filter_field
{
    uint32 fieldType;
    
    union
    {
        /* L2 struct */
        rtk_filter_mac_t       dmac;
        rtk_filter_mac_t       smac;
        rtk_filter_value_t     etherType;
        rtk_filter_tag_t       ctag;
        rtk_filter_tag_t       relayCtag;
        rtk_filter_tag_t       stag;
        rtk_filter_dot1as_timestamp_t dot1asTimeStamp;
        
        /* L3 struct */
	      rtk_filter_ip_t      sip;
        rtk_filter_ip_t      dip;
        rtk_filter_value_t   protocol;
        rtk_filter_value_t   ipTos;
        rtk_filter_ipFlag_t  ipFlag;
        rtk_filter_value_t   ipOffset;
	      rtk_filter_ip6_t     sipv6;
        rtk_filter_ip6_t     dipv6;
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
        rtk_filter_value_t   udpDatcPort;
        rtk_filter_value_t   icmpCode;
        rtk_filter_value_t   icmpType;
        rtk_filter_value_t   igmpType;

        /* pattern match */
        rtk_filter_pattern_t pattern;
    } filter_pattern_union;

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
    FILTER_FIELD_MAX,
} rtk_filter_field_type_t;

typedef enum rtk_filter_field_type_raw_e
{
    FILTER_FIELD_RAW_DMAC_47_32 = 0,
    FILTER_FIELD_RAW_DMAC_31_16,
    FILTER_FIELD_RAW_DMAC_15_0,    
    FILTER_FIELD_RAW_SMAC_47_32,
    FILTER_FIELD_RAW_SMAC_31_16,
    FILTER_FIELD_RAW_SMAC_15_0,    
    FILTER_FIELD_RAW_ETHERTYPE,
    FILTER_FIELD_RAW_CTAG,
    FILTER_FIELD_RAW_STAG, 
    
    FILTER_FIELD_RAW_IPV4_SIP_31_16,
    FILTER_FIELD_RAW_IPV4_SIP_15_0,    
    FILTER_FIELD_RAW_IPV4_DIP_31_16,
    FILTER_FIELD_RAW_IPV4_DIP_15_0,    
    FILTER_FIELD_RAW_IPV4_TOS_PROTOCOL,
    FILTER_FIELD_RAW_IPV4_FLAG_OFFSET,
    FILTER_FIELD_RAW_TCP_SPORT,
    FILTER_FIELD_RAW_TCP_DPORT,
    FILTER_FIELD_RAW_TCP_FLAG,
    FILTER_FIELD_RAW_UDP_SPORT,
    FILTER_FIELD_RAW_UDP_DPORT,    
    FILTER_FIELD_RAW_ICMP_CODE_TYPE,
    FILTER_FIELD_RAW_IGMP_TYPE,

    FILTER_FIELD_RAW_IPV6_SIP_15_0,
    FILTER_FIELD_RAW_IPV6_SIP_31_16,
    FILTER_FIELD_RAW_IPV6_SIP_47_32,
    FILTER_FIELD_RAW_IPV6_SIP_63_48,
    FILTER_FIELD_RAW_IPV6_SIP_79_64,
    FILTER_FIELD_RAW_IPV6_SIP_95_80,
    FILTER_FIELD_RAW_IPV6_SIP_111_96,     
    FILTER_FIELD_RAW_IPV6_SIP_127_112,

    FILTER_FIELD_RAW_IPV6_DIP_15_0,
    FILTER_FIELD_RAW_IPV6_DIP_31_16,
    FILTER_FIELD_RAW_IPV6_DIP_47_32,
    FILTER_FIELD_RAW_IPV6_DIP_63_48,
    FILTER_FIELD_RAW_IPV6_DIP_79_64,
    FILTER_FIELD_RAW_IPV6_DIP_95_80,
    FILTER_FIELD_RAW_IPV6_DIP_111_96,     
    FILTER_FIELD_RAW_IPV6_DIP_127_112,
    FILTER_FIELD_RAW_IPV6_TRAFFIC_CLASS_NEXT_HEADER,

    FILTER_FIELD_RAW_MAX,
} rtk_filter_field_type_raw_t;


typedef enum rtk_filter_flag_care_type_e
{
    FILTER_FLAG_CARE_DONT_CARE = 0,
    FILTER_FLAG_CARE_1,
    FILTER_FLAG_CARE_0,
    FILTER_FLAG_END
} rtk_filter_flag_care_type_t;

typedef uint32  rtk_filter_id_t;    /* filter id type */
typedef uint32  rtk_filter_template_index_t;

typedef enum rtk_filter_invert_e
{
    FILTER_INVERT_DISABLE = 0,
    FILTER_INVERT_ENABLE,
    FILTER_INVERT_END,
} rtk_filter_invert_t;

typedef uint32 rtk_filter_state_t;

typedef uint32 rtk_filter_unmatch_action_t;

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
    rtk_filter_field_raw_t      dataFieldRaw[RTK_MAX_NUM_OF_FILTER_FIELD];
    rtk_filter_field_raw_t      careFieldRaw[RTK_MAX_NUM_OF_FILTER_FIELD];
    rtk_filter_field_type_raw_t fieldRawType[RTK_MAX_NUM_OF_FILTER_FIELD];
    rtk_filter_care_tag_t       careTag;
    rtk_filter_value_t          activeport;	
    	
    rtk_filter_invert_t         invert;
	rtk_enable_t                valid;
} rtk_filter_cfg_raw_t;

typedef struct
{
    uint32 index;
    rtk_filter_field_type_raw_t fieldType[RTK_MAX_NUM_OF_FILTER_FIELD];
} rtk_filter_template_t;


typedef enum rtk_igmp_type_e
{
    IGMP_IPV4 = 0,
    IGMP_PPPOE_IPV4,
    IGMP_MLD,
    IGMP_PPPOE_MLD,
    IGMP_TYPE_END
} rtk_igmp_type_t;

typedef uint32 rtk_int_info_t;

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
    ADV_METER32_47,_MASK,
    ADV_METER48_63_MASK,
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
    uint8 value[RTK_MAX_NUM_OF_INTERRUPT_TYPE];
} rtk_int_status_t;

typedef uint32 rtk_ipaddr_t;

typedef enum rtk_l2_age_time_e
{
    AGE_TIME_300S= 0,
    AGE_TIME_2,
    AGE_TIME_3,
    AGE_TIME_4,
    AGE_TIME_END
} rtk_l2_age_time_t;

typedef enum rtk_l2_flood_type_e
{
    FLOOD_UNKNOWNDA = 0,
    FLOOD_UNKNOWNMC,
    FLOOD_BC,
    FLOOD_END
} rtk_l2_flood_type_t;

typedef uint32 rtk_l2_flushItem_t;

typedef enum rtk_l2_flushType_e
{
    FLUSH_TYPE_BY_PORT = 0,       /* physical port       */
    FLUSH_TYPE_END
} rtk_l2_flushType_t;


typedef enum rtk_l2_hash_method_e
{
    HSAH_OPT0 = 0,
    HASH_OPT1,
    HASH_END
} rtk_hash_method_t;

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
    rtk_mac_t   mac;
    uint32      fid;
    uint32      port;
    uint32      sa_block;
    uint32      auth;
    uint32      is_static;
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
	int32 	index;
	ipaddr_t sip;
	ipaddr_t dip;
	rtk_mac_t mac;	
	uint32 	sa_block;
	uint32 	auth;
	uint32 	portmask;
	uint32 	age;
	uint32 	fid;
	uint32 	is_ipmul;
	uint32 	is_static;    
}rtk_l2_addr_table_t;


typedef uint32  rtk_mac_cnt_t;     /* meter id type  */

typedef enum rtk_mcast_type_e
{
    MCAST_L2 = 0,
    MCAST_IPV4,
    MCAST_IPV6,
    MCAST_END
} rtk_mcast_type_t;

typedef uint32  rtk_meter_id_t;     /* meter id type  */

typedef enum rtk_mode_e
{
    MODE0 = 0,
    MODE1,
    MODE_END
} rtk_mode_t;


typedef uint32  rtk_port_t;        /* port is type */

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
    uint32 forcemode;
    uint32 speed;
    uint32 duplex;
    uint32 link;    
    uint32 nway;    
    uint32 txpause;
    uint32 rxpause;      
    uint32 lpi100;
    uint32 lpi1000;   
}rtk_port_mac_ability_t;

typedef struct rtk_port_phy_ability_s
{   
    uint32    AutoNegotiation;  /*PHY register 0.12 setting for auto-negotiation process*/
    uint32    Half_10;          /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    uint32    Full_10;          /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    uint32    Half_100;         /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    uint32    Full_100;         /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    uint32    Full_1000;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    uint32    FC;               /*PHY register 4.10 setting for flow control capability*/
    uint32    AsyFC;            /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtk_port_phy_ability_t;

typedef uint32  rtk_port_phy_data_t;     /* phy page  */

typedef uint32  rtk_port_phy_page_t;     /* phy page  */

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

typedef struct rtk_portmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef uint32  rtk_pri_t;         /* priority vlaue */

typedef struct rtk_priority_select_s
{   
    uint32 port_pri;
    uint32 dot1q_pri;
    uint32 acl_pri;
    uint32 dscp_pri;
    uint32 cvlan_pri;
    uint32 svlan_pri;
    uint32 dmac_pri;
    uint32 smac_pri;
} rtk_priority_select_t;


typedef uint32  rtk_qid_t;        /* queue id type */

typedef struct rtk_qos_pri2queue_s
{
    uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
} rtk_qos_pri2queue_t;

typedef struct rtk_qos_queue_weights_s
{
    uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef enum rtk_qos_scheduling_type_e
{
    WFQ = 0,        /* Weighted-Fair-Queue */
    WRR,            /* Weighted-Round-Robin */
    SCHEDULING_TYPE_END
} rtk_qos_scheduling_type_t;

typedef uint32  rtk_queue_num_t;    /* queue number*/

typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_END
} rtk_rate_storm_group_t;

typedef uint32  rtk_rate_t;     /* rate type  */

typedef rtk_u_long_t rtk_stat_counter_t;

#ifndef EMBEDDED_SUPPORT
/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    uint64 dot1dTpLearnedEntryDiscards;
}rtk_stat_global_cntr_t;
#endif

typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 36,
    MIB_GLOBAL_CNTR_END
}rtk_stat_global_type_t;

#ifndef EMBEDDED_SUPPORT
/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    uint64 ifInOctets;
    uint32 dot3StatsFCSErrors;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3InPauseFrames;
    uint32 dot3ControlInUnknownOpcodes;
    uint32 etherStatsFragments;
    uint32 etherStatsJabbers;
    uint32 ifInUcastPkts;
    uint32 etherStatsDropEvents;
    uint64 etherStatsOctets;
    uint32 etherStatsUndersizePkts;
    uint32 etherStatsOversizePkts;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024toMaxOctets;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsBcastPkts;	
    uint64 ifOutOctets;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsLateCollisions;
    uint32 etherStatsCollisions;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3OutPauseFrames;
    uint32 dot1dBasePortDelayExceededDiscards;
    uint32 dot1dTpPortInDiscards;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
    uint32 outOampduPkts;
    uint32 inOampduPkts;
    uint32 pktgenPkts;
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
    STAT_PORT_CNTR_END
}rtk_stat_port_type_t;

typedef uint32  rtk_stg_t;        /* spanning tree instance id type */

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

typedef uint32  rtk_stp_msti_id_t;     /* MSTI ID  */

typedef enum rtk_stp_state_e
{
    STP_STATE_DISABLED = 0,
    STP_STATE_BLOCKING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_END
} rtk_stp_state_t;

typedef uint32 rtk_svlan_index_t;

typedef struct rtk_svlan_memberCfg_s{
    uint32 svid;   
    uint32 memberport;
    uint32 fid;
    uint32 priority;
    uint32 efiden; 
    uint32 efid;     
    uint32 reserved3; 
    uint32 reserved4;   
}rtk_svlan_memberCfg_t;

typedef enum rtk_svlan_pri_ref_e
{
    REF_INTERNAL_PRI = 0,
    REF_CTAG_PRI,
    REF_SVLAN_PRI,
    REF_PRI_END
} rtk_svlan_pri_ref_t;


typedef uint32 rtk_svlan_tpid_t;

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
    IGMP_ACTION_END
} rtk_trap_igmp_action_t;

typedef enum rtk_trap_mcast_action_e
{
    MCAST_ACTION_FORWARD = 0,
    MCAST_ACTION_DROP,
    MCAST_ACTION_TRAP2CPU,
    MCAST_ACTION_END
} rtk_trap_mcast_action_t;

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
    uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef uint32  rtk_vlan_proto_type_t;     /* protocol and port based VLAN protocol type  */


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

typedef uint32  rtk_vlan_t;        /* vlan id type */

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    uint32                         proto_type;
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

typedef enum rtk_rldp_transmitMode_e
{
    RLDP_TRANSMIT_MODE_0 = 0,
    RLDP_TRANSMIT_MODE_1,
    RLDP_TRANSMIT_MODE_END
} rtk_rldp_transmitMode_t;

typedef struct rtk_rtctResult_s
{
    rtk_port_speed_t    linkType;
    union
    {
        struct fe_result_s
        {
            uint32      isRxShort;
            uint32      isTxShort;
            uint32      isRxOpen;
            uint32      isTxOpen;
            uint32      isRxMismatch;
            uint32      isTxMismatch;
            uint32      isRxLinedriver;
            uint32      isTxLinedriver;
            uint32      rxLen;
            uint32      txLen;
        } fe_result;

        struct ge_result_s
        {
            uint32      channelAShort;
            uint32      channelBShort;
            uint32      channelCShort;
            uint32      channelDShort;

            uint32      channelAOpen;
            uint32      channelBOpen;
            uint32      channelCOpen;
            uint32      channelDOpen;

            uint32      channelAMismatch;
            uint32      channelBMismatch;
            uint32      channelCMismatch;
            uint32      channelDMismatch;

            uint32      channelALinedriver;
            uint32      channelBLinedriver;
            uint32      channelCLinedriver;
            uint32      channelDLinedriver;

            uint32      channelALen;
            uint32      channelBLen;
            uint32      channelCLen;
            uint32      channelDLen;
        } ge_result;
    };
} rtk_rtctResult_t;

#endif /* __RTK_API_H__ */

