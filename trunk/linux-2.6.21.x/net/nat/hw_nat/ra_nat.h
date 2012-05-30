/*
    Module Name:
    ra_nat.h

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
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_ether.h>
#include <linux/netfilter.h>
#include <linux/etherdevice.h>

/*
 * TYPEDEFS AND STRUCTURES
 */
enum DstPort {
	DP_RA0 = 0,
#if defined (CONFIG_RT2860V2_AP_MBSS)
	DP_RA1 = 1,
	DP_RA2 = 2,
	DP_RA3 = 3,
	DP_RA4 = 4,
	DP_RA5 = 5,
	DP_RA6 = 6,
	DP_RA7 = 7,
	DP_RA8 = 8,
	DP_RA9 = 9,
	DP_RA10 = 10,
	DP_RA11 = 11,
	DP_RA12 = 12,
	DP_RA13 = 13,
	DP_RA14 = 14,
	DP_RA15 = 15,
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	DP_WDS0 = 16,
	DP_WDS1 = 17,
	DP_WDS2 = 18,
	DP_WDS3 = 19,
#endif // CONFIG_RT2860V2_AP_WDS //
#if defined (CONFIG_RT2860V2_AP_APCLI)
	DP_APCLI0 = 20,
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	DP_MESH0 = 21,
#endif // CONFIG_RT2860V2_AP_MESH //
	DP_RAI0 = 24,
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)

	DP_RAI1 = 25,
	DP_RAI2 = 26,
	DP_RAI3 = 27,
	DP_RAI4 = 28,
	DP_RAI5 = 29,
	DP_RAI6 = 30,
	DP_RAI7 = 31,
	DP_RAI8 = 32,
	DP_RAI9 = 33,
	DP_RAI10 = 34,
	DP_RAI11 = 35,
	DP_RAI12 = 36,
	DP_RAI13 = 37,
	DP_RAI14 = 38,
	DP_RAI15 = 39,
#endif // CONFIG_RTDEV_AP_MBSS //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS) || \
    defined (CONFIG_RT5592_AP_WDS) || defined (CONFIG_RT3593_AP_WDS)
	DP_WDSI0 = 40,
	DP_WDSI1 = 41,
	DP_WDSI2 = 42,
	DP_WDSI3 = 43,
#endif // CONFIG_RTDEV_AP_WDS //
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)
	DP_APCLII0 = 44,
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
	DP_MESHI0 = 45,
#endif // CONFIG_RTDEV_AP_MESH //
	DP_GMAC = 50,
	DP_GMAC2 = 51,
	DP_PCI = 52,
	MAX_IF_NUM
};

typedef struct {
#if defined (CONFIG_HNAT_V2)
	uint16_t MAGIC_TAG;
	uint32_t FOE_Entry:14;
	uint32_t CRSN:5;
	uint32_t SPORT:3;
	uint32_t ALG:10;
#else
	uint16_t MAGIC_TAG;
	uint32_t FOE_Entry:14;
	uint32_t FVLD:1;
	uint32_t ALG:1;
	uint32_t AI:8;
	uint32_t SP:3;
	uint32_t AIS:1;
	uint32_t RESV2:4;
#endif
}  __attribute__ ((packed)) PdmaRxDescInfo4;

typedef struct {
	//layer2 header
	uint8_t dmac[6];
	uint8_t smac[6];

	//vlan header 
	uint16_t vlan_tag;
	uint16_t vlan1_gap;
	uint16_t vlan1;
	uint16_t vlan2_gap;
	uint16_t vlan2;
	uint16_t vlan_layer;

	//pppoe header
	uint32_t pppoe_gap;
	uint16_t ppp_tag;
	uint16_t pppoe_sid;

	//layer3 header
	uint16_t eth_type;
	struct iphdr iph;
	struct ipv6hdr ip6h;

	//layer4 header
	struct tcphdr th;
	struct udphdr uh;

	uint32_t pkt_type;

} PktParseResult;


/*
 * DEFINITIONS AND MACROS
 */
#ifndef NEXTHDR_IPIP
#define NEXTHDR_IPIP 4
#endif

/*
 *    2bytes	    4bytes 
 * +-----------+-------------------+
 * | Magic Tag | RX/TX Desc info4  |
 * +-----------+-------------------+
 * |<------FOE Flow Info---------->|
 */
#define FOE_INFO_LEN		    6
#define FOE_MAGIC_PCI		    0x7273
#define FOE_MAGIC_WLAN		    0x7274
#define FOE_MAGIC_GE		    0x7275
#define FOE_MAGIC_PPE		    0x7276

/* choose one of them to keep HNAT related information in somewhere. */
#define HNAT_USE_HEADROOM
//#define HNAT_USE_TAILROOM
//#define HNAT_USE_SKB_CB

#if defined (HNAT_USE_HEADROOM)
#define IS_SPACE_AVAILABLED(skb)    ((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->head)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SPORT	//src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_TAILROOM)

#define IS_SPACE_AVAILABLED(skb)    ((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->end - FOE_INFO_LEN)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SPORT //src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_SKB_CB)
//change the position of skb_CB if necessary
#define CB_OFFSET		    32
#define IS_SPACE_AVAILABLED(skb)    1
#define FOE_INFO_START_ADDR(skb)    (skb->cb +  CB_OFFSET)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->SPORT	//src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)(&(skb)->cb + CB_OFFSET))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)(&(skb)->cb + CB_OFFSET))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb + CB_OFFSET))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb + CB_OFFSET))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)(&(skb)->cb + CB_OFFSET))->SP	//src_port or user priority
#endif

#endif

#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE)   || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER2_HEADER(skb)		(skb)->mac_header
#else
#define LAYER2_HEADER(skb)		(skb)->mac.raw
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER3_HEADER(skb)		(skb)->network_header
#else
#define LAYER3_HEADER(skb)		(skb)->nh.raw
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER4_HEADER(skb)		(skb)->transport_header
#else
#define LAYER4_HEADER(skb)		(skb)->h.raw
#endif

/*
 * EXPORT FUNCTION
 */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, uint16_t * sid, uint16_t * ppp_tag);

int PpeSetBindThreshold(uint32_t threshold);
int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt);
int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac,
		   uint16_t post_meter, uint16_t post_ac);

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka);
int PpeSetUnbindLifeTime(uint8_t lifetime);
int PpeSetBindLifetime(uint16_t tcp_fin, uint16_t udp_life, uint16_t fin_life);

#endif
