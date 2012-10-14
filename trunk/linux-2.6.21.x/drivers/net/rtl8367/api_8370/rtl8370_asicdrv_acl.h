#ifndef _RTL8370_ASICDRV_ACL_H_
#define _RTL8370_ASICDRV_ACL_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_ACLRULENO                       64
#define RTL8370_ACLRULEMAX                      (RTL8370_ACLRULENO-1)
#define RTL8370_ACLTYPEFIELDMAX                 7
#define RTL8370_ACLTYPENO                       5
#define RTL8370_ACLTYPEMAX                      (RTL8370_ACLTYPENO-1)

#define RTL8370_ACLRULETBLEN                    9
#define RTL8370_ACLACTTBLEN                     3
#define RTL8370_ACLRULETBADDR(type, rule)       ((type << 7) | rule)

#define ACL_ACT_CVLAN_ENABLE_MASK               0x1
#define ACL_ACT_SVLAN_ENABLE_MASK               0x2
#define ACL_ACT_PRIORITY_ENABLE_MASK            0x4
#define ACL_ACT_POLICING_ENABLE_MASK            0x8
#define ACL_ACT_FWD_ENABLE_MASK                 0x10

enum ACLTCAMTYPES
{
    CAREBITS= 0,
    DATABITS
};

typedef enum aclFwdAct
{
    RTL8370_ACL_FWD_MIRROR = 0,
    RTL8370_ACL_FWD_REDIRECT,
    RTL8370_ACL_FWD_MIRRORFUNTION,
    RTL8370_ACL_FWD_TRAP,
} rtl8370_aclFwd_t;

enum ACLFIELDTYPES
{
    DMAC0 = 0,
    DMAC1,
    DMAC2,
    SMAC0,
    SMAC1,
    SMAC2,
    ETHERTYPE,
    STAG,
    CTAG,
    IP4SIP0,
    IP4SIP1,
    IP4DIP0,
    IP4DIP1,
    IP4TOSPROTO,
    IP4FLAGOFF,
    TCPSPORT,
    TCPDPORT,
    TCPFLAG,
    UDPSPORT,
    UDPDPORT,
    ICMPCODETYPE,
    IGMPTYPE,
    IP6SIP0,
    IP6SIP1,
    IP6SIP2,
    IP6SIP3,
    IP6SIP4,
    IP6SIP5,
    IP6SIP6,
    IP6SIP7,
    IP6DIP0,
    IP6DIP1,
    IP6DIP2,
    IP6DIP3,
    IP6DIP4,
    IP6DIP5,
    IP6DIP6,
    IP6DIP7,
    TOSNH,
    TYPE_MAX
};

typedef struct
{
#ifdef _LITTLE_ENDIAN
    uint16 active_portmsk;

    uint16 type:3;
    uint16 tag_exist:9;
    uint16 reserved:4;
#if 0
    uint16 field0_0:8;    
    uint16 field0_1:8;
    uint16 field1_0:8;
    uint16 field1_1:8;
    uint16 field2_0:8;
    uint16 field2_1:8;
    uint16 field3_0:8;
    uint16 field3_1:8;
    uint16 field4_0:8;
    uint16 field4_1:8;
    uint16 field5_0:8;
    uint16 field5_1:8;
    uint16 field6_0:8;
    uint16 field6_1:8;
#endif
    uint16 field[RTL8370_ACLTYPEFIELDMAX];
#else
    uint16 active_portmsk;

    uint16 reserved:4;
    uint16 tag_exist:9;
    uint16 type:3;
#if 0
    uint16 field0_1:8;     
    uint16 field0_0:8;
    uint16 field1_1:8;
    uint16 field1_0:8;
    uint16 field2_1:8;
    uint16 field2_0:8;
    uint16 field3_1:8;
    uint16 field3_0:8;
    uint16 field4_1:8;
    uint16 field4_0:8;
    uint16 field5_1:8;
    uint16 field5_0:8;
    uint16 field6_1:8;
    uint16 field6_0:8;
#endif
    uint16 field[RTL8370_ACLTYPEFIELDMAX];
#endif
} rtl8370_acl_pattern_smi_t;  

typedef struct 
{
    rtl8370_acl_pattern_smi_t  care_bits;
    rtl8370_acl_pattern_smi_t  data_bits;
    uint32        valid;
} rtl8370_acl_rule_smi_t;

typedef struct
{
    uint32 pri;
    uint32 cfi;
    uint32 vid;
}  tag_t;

typedef struct
{
    uint32 tos;
    uint32 proto;
}  ip4_tos_proto_t;

typedef struct
{
    uint32 df;
    uint32 mf;
    uint32 offset;
}  ip4_flag_off_t;

typedef struct
{
    uint32 syn;
    uint32 ack;
    uint32 fin;
    uint32 rst;
    uint32 urg;
    uint32 psh;
} tcp_flag_t;

typedef struct
{
    uint32 code_value;
    uint32 type;
} icmp_code_type_t;

typedef struct
{
    uint32 type;
    union 
    {
        ether_addr_t        dmac;
        ether_addr_t        smac;        
        uint32              etherType;
        tag_t               ctag;
        tag_t               stag;
        ipaddr_t            sip;
        ipaddr_t            dip;
        ip4_tos_proto_t     ip4TosPro;
        uint32              tcpSrcPort;
        uint32              tcpDstPort;
        tcp_flag_t          tcpFlag;
        uint32              udpSrcPort;
        uint32              udpDstPort;
        icmp_code_type_t    icmp;
        uint32              igmp_type;
    } acl_field_type_union;
} rtl8370_acl_pattern_api_t;

typedef struct
{
    uint32 active_portmsk;
    uint32 type;
    uint32 tag_exist;
    uint32                 field[RTL8370_ACLTYPEFIELDMAX];
} rtl8370_acl_pattern_t;

typedef struct 
{
    rtl8370_acl_pattern_t    data_bits;
    rtl8370_acl_pattern_t    care_bits;
    uint32                   valid;
} rtl8370_acl_rule_t;


typedef struct
{
    uint32 field[7];
} rtl8370_acl_template_t;


typedef struct 
{
#ifdef _LITTLE_ENDIAN
    uint16 aclcvid:12;
    uint16 ct:1;
    uint16 mrat:2;
    uint16 arpmsk_1:1;

    uint16 arpmsk_2:15;
    uint16 pri_1:1;

    uint16 pri_2:2;
    uint16 aclsvidx:6;    
    uint16 aclmeteridx:8;
#else
    uint16 arpmsk_1:1;    
    uint16 mrat:2;
    uint16 ct:1;    
      uint16 aclcvid:12;        

    uint16 pri_1:1;    
    uint16 arpmsk_2:15;

    uint16 aclmeteridx:8;    
    uint16 aclsvidx:6;       
    uint16 pri_2:2;    
#endif
} rtl8370_acl_act_smi_t;

typedef struct
{
    uint32 arpmsk;
    uint32 mrat;
    uint32 aclmeteridx;
    uint32 aclsvidx;
    uint32 aclpri;
    uint32 aclcvid;
    uint32 ct;    
} rtl8370_acl_act_t;

typedef struct
{
    rtl8370_acl_rule_t aclRule; 
    rtl8370_acl_act_t  aclAct;
    uint32              aclActCtrl;
    uint32              aclNot;
} rtl8370_acl_rule_union_t;


extern ret_t rtl8370_setAsicAcl(uint32 port, uint32 enabled);
extern ret_t rtl8370_getAsicAcl(uint32 port, uint32* enabled);
extern ret_t rtl8370_setAsicAclUnmatchedPermit(uint32 port, uint32 enabled);
extern ret_t rtl8370_getAsicAclUnmatchedPermit(uint32 port, uint32* enabled);
extern ret_t rtl8370_setAsicAclRule(uint32 index, rtl8370_acl_rule_t *aclRule);
extern ret_t rtl8370_getAsicAclRule(uint32 index, rtl8370_acl_rule_t *aclRule);
extern ret_t rtl8370_setAsicAclNot(uint32 index, uint32 not);
extern ret_t rtl8370_getAsicAclNot(uint32 index, uint32* not);
extern ret_t rtl8370_setAsicAclType(uint32 index, rtl8370_acl_template_t aclType);
extern ret_t rtl8370_getAsicAclType(uint32 index, rtl8370_acl_template_t *aclType);
extern ret_t rtl8370_setAsicAclAct(uint32 index, rtl8370_acl_act_t aclAct);
extern ret_t rtl8370_getAsicAclAct( uint32 index, rtl8370_acl_act_t *aclAct);
extern ret_t rtl8370_setAsicAclActCtrl( uint32 index, uint32 aclActCtrl);
extern ret_t rtl8370_getAsicAclActCtrl( uint32 index, uint32 *aclActCtrl);

#endif /*_RTL8370_ASICDRV_ACL_H_*/


