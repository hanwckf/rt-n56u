/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    foe_fdb.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef _RA_NAT_WANTED
#define _RA_NAT_WANTED

#include "foe_fdb.h"
#include <linux/etherdevice.h>
#include <linux/if_ether.h>

/*
 * TYPEDEFS AND STRUCTURES
 */
enum DstPort {
    DP_RA0=0,
#if defined (CONFIG_RT2860V2_AP_MBSS)
    DP_RA1=1,
    DP_RA2=2,
    DP_RA3=3,
    DP_RA4=4,
    DP_RA5=5,
    DP_RA6=6,
    DP_RA7=7,
    DP_RA8=8,
    DP_RA9=9,
    DP_RA10=10,
    DP_RA11=11,
    DP_RA12=12,
    DP_RA13=13,
    DP_RA14=14,
    DP_RA15=15,
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
    DP_WDS0=16,
    DP_WDS1=17,
    DP_WDS2=18,
    DP_WDS3=19,
#endif // CONFIG_RT2860V2_AP_WDS //
#if defined (CONFIG_RT2860V2_AP_APCLI)
    DP_APCLI0=20,
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
    DP_MESH0=21,
#endif // CONFIG_RT2860V2_AP_MESH //
    DP_RAI0=24,
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS)
    DP_RAI1=25,
    DP_RAI2=26,
    DP_RAI3=27,
    DP_RAI4=28,
    DP_RAI5=29,
    DP_RAI6=30,
    DP_RAI7=31,
    DP_RAI8=32,
    DP_RAI9=33,
    DP_RAI10=34,
    DP_RAI11=35,
    DP_RAI12=36,
    DP_RAI13=37,
    DP_RAI14=38,
    DP_RAI15=39,
#endif // CONFIG_RTDEV_AP_MBSS //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS)
    DP_WDSI0=40,
    DP_WDSI1=41,
    DP_WDSI2=42,
    DP_WDSI3=43,
#endif // CONFIG_RTDEV_AP_WDS //
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI)
    DP_APCLII0=44,
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH)
    DP_MESHI0=45,
#endif // CONFIG_RTDEV_AP_MESH //
    DP_GMAC=50,
    DP_GMAC2=51,
    DP_PCI=52,
    MAX_IF_NUM
};

typedef struct 
{
    uint16_t  MAGIC_TAG;
    uint32_t  FOE_Entry: 14;
    uint32_t  FVLD:1;
    uint32_t  ALG:1;
    uint32_t  AI:8;
    uint32_t  SP:3;
    uint32_t  AIS:1;
    uint32_t  RESV:4; 
} PdmaRxDescInfo4;

#define FOE_INFO_LEN	6

/*
 * DEFINITIONS AND MACROS
 */

/*
 *    2bytes	    4bytes 
 * +-----------+-------------------+
 * | Magic Tag | RX/TX Desc info4  |
 * +-----------+-------------------+
 * |<------FOE Flow Info---------->|
 */
#define FOE_MAGIC_PCI		    0x7273
#define FOE_MAGIC_WLAN		    0x7274
#define FOE_MAGIC_GE		    0x7275
#define FOE_MAGIC_PPE		    0x7276

/* choose one of them to keep HNAT related information in somewhere. */
#define HNAT_USE_HEADROOM
//#define HNAT_USE_TAILROOM
//#define HNAT_USE_SKB_CB

#if defined (HNAT_USE_HEADROOM)
#define IS_SPACE_AVAILABLED(skb)    (skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0
#define FOE_INFO_START_ADDR(skb)    (skb->head)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#define FOE_FVLD(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->FVLD
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SP //src_port or user priority
#define FOE_AIS(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->AIS
#elif defined (HNAT_USE_TAILROOM)
#define IS_SPACE_AVAILABLED(skb)    (skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0
#define FOE_INFO_START_ADDR(skb)    (skb->end - FOE_INFO_LEN)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#define FOE_FVLD(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FVLD
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SP //src_port or user priority
#define FOE_AIS(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->AIS
#elif defined (HNAT_USE_SKB_CB)
//change the position of skb_CB if necessary
#define IS_SPACE_AVAILABLED(skb)    1
#define FOE_INFO_START_ADDR(skb)    (skb->cb + 40)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->FOE_Entry
#define FOE_FVLD(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->FVLD
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->SP //src_port or user priority
#define FOE_AIS(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb[40]))->AIS
#endif

#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI)	|| \
				     (FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE)	|| \
				     (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))

/*
 * EXPORT FUNCTION
 */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, uint16_t *sid, uint16_t *ppp_tag, uint32_t is_in);
int PpeSetDscpRemarkEbl(uint32_t enable);
int PpeSetVpriRemarkEbl(uint32_t enable);
int PpeSetWeightFOE(uint32_t weight);
int PpeSetWeightACL(uint32_t weight);
int PpeSetWeightDSCP(uint32_t weight);
int PpeSetWeightVPRI(uint32_t weight);
int PpeSetDSCP_UP(uint32_t DSCP_SET, unsigned char UP);
int PpeSetUP_IDSCP(uint32_t UP, uint32_t IDSCP);
int PpeSetUP_ODSCP(uint32_t UP, uint32_t ODSCP);
int PpeSetUP_VPRI(uint32_t UP, uint32_t VPRI);
int PpeSetUP_AC(uint32_t UP, uint32_t AC);
int PpeSetSchMode(uint32_t policy);
int PpeSetSchWeight(uint8_t W0, uint8_t W1, uint8_t W2, uint8_t W3);

int PpeSetBindThreshold(uint32_t threshold);
int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt);
int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac, uint16_t post_meter, uint16_t post_ac);

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka);
int PpeSetUnbindLifeTime(uint8_t lifetime);
int PpeSetBindLifetime(uint16_t tcp_fin, uint16_t udp_life, uint16_t fin_life);

#endif
