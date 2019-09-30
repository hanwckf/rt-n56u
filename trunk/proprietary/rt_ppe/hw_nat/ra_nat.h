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

#define HW_NAT_MODULE_VER "v2.52.0"

#if !defined (CONFIG_RT_SECOND_IF_NONE) && !defined (CONFIG_RT_SECOND_IF_RT3352_INIC)
#define HWNAT_USE_SECOND_IF
#endif

/*
 * TYPEDEFS AND STRUCTURES
 */
enum DstPort {
	DP_RA0 = 11,
	DP_RA1 = 12,
	DP_RA2 = 13,
	DP_RA3 = 14,
	DP_RA4 = 15,
	DP_RA5 = 16,
	DP_RA6 = 17,
	DP_RA7 = 18,
	DP_RA8 = 19,
	DP_RA9 = 20,
	DP_RA10 = 21,
	DP_RA11 = 22,
	DP_RA12 = 23,
	DP_RA13 = 24,
	DP_RA14 = 25,
	DP_RA15 = 26,
	DP_WDS0 = 27,
	DP_WDS1 = 28,
	DP_WDS2 = 29,
	DP_WDS3 = 30,
	DP_APCLI0 = 31,
	DP_MESH0 = 32,
#if defined (HWNAT_USE_SECOND_IF)
	DP_RAI0 = 33,
	DP_RAI1 = 34,
	DP_RAI2 = 35,
	DP_RAI3 = 36,
	DP_RAI4 = 37,
	DP_RAI5 = 38,
	DP_RAI6 = 39,
	DP_RAI7 = 40,
	DP_RAI8 = 41,
	DP_RAI9 = 42,
	DP_RAI10 = 43,
	DP_RAI11 = 44,
	DP_RAI12 = 45,
	DP_RAI13 = 46,
	DP_RAI14 = 47,
	DP_RAI15 = 48,
	DP_WDSI0 = 49,
	DP_WDSI1 = 50,
	DP_WDSI2 = 51,
	DP_WDSI3 = 52,
	DP_APCLII0 = 53,
	DP_MESHI0 = 54,
	MAX_WIFI_IF_NUM = 55,
#elif defined (HWNAT_USE_IF_DBDC)
	DP_RAX0 = 33,
	DP_RAX1 = 34,
	DP_RAX2 = 35,
	DP_RAX3 = 36,
	DP_RAX4 = 37,
	DP_RAX5 = 38,
	DP_RAX6 = 39,
	DP_RAX7 = 40,
	DP_RAX8 = 41,
	DP_RAX9 = 42,
	DP_RAX10 = 43,
	DP_RAX11 = 44,
	DP_RAX12 = 45,
	DP_RAX13 = 46,
	DP_RAX14 = 47,
	DP_RAX15 = 48,
	DP_WDSX0 = 49,
	DP_WDSX1 = 50,
	DP_WDSX2 = 51,
	DP_WDSX3 = 52,
	DP_APCLIX0 = 53,
	DP_MESHX0 = 54,
	MAX_WIFI_IF_NUM = 55,
#else
	MAX_WIFI_IF_NUM = 33,
#endif
	DP_GMAC1 = 60,
	DP_GMAC2 = 61,
	DP_NIC0 = 62,
	DP_NIC1 = 63,
	MAX_IF_NUM // MAX_IF_NUM = 64 entries (act_dp length 6bits)
};

typedef struct {
	uint16_t MAGIC_TAG;
	uint32_t FOE_Entry:14;
#if defined (CONFIG_RALINK_MT7620)
	uint32_t CRSN:5;
	uint32_t SPORT:3;
	uint32_t ALG:10;
#elif defined (CONFIG_RALINK_MT7621)
	uint32_t CRSN:5;
	uint32_t SPORT:4;
	uint32_t ALG:9;
#else
	uint32_t FVLD:1;
	uint32_t ALG:1;
	uint32_t AI:8;
	uint32_t SP:3;
	uint32_t AIS:1;
	uint32_t RESV2:4;
#endif
}  __attribute__ ((packed)) PdmaRxDescInfo4;

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
#define FOE_MAGIC_EXTIF		    0x7274
#define FOE_MAGIC_GE		    0x7275
#define FOE_MAGIC_PPE		    0x7276
#define FOE_MAGIC_PPE_DWORD	    0x3fff7276UL	/* HNAT_V1: FVLD=0, HNAT_V2: FOE_Entry=0x3fff */

/* choose one of them to keep HNAT related information in somewhere. */
#define HNAT_USE_HEADROOM
//#define HNAT_USE_TAILROOM
//#define HNAT_USE_SKB_CB

#if defined (HNAT_USE_HEADROOM)
#define IS_SPACE_AVAILABLED(skb)    ((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->head)

#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#if defined (CONFIG_HNAT_V2)
#define FOE_ENTRY_VALID(skb)	    (((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry != 0x3fff)
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SPORT
#else
#define FOE_ENTRY_VALID(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FVLD
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_TAILROOM)
#define IS_SPACE_AVAILABLED(skb)    ((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->end - FOE_INFO_LEN)

#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#if defined (CONFIG_HNAT_V2)
#define FOE_ENTRY_VALID(skb)	    (((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry != 0x3fff)
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SPORT
#else
#define FOE_ENTRY_VALID(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FVLD
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_SKB_CB)
//change the position of skb_CB if necessary
#define CB_OFFSET		    32
#define IS_SPACE_AVAILABLED(skb)    1
#define FOE_INFO_START_ADDR(skb)    (skb->cb + CB_OFFSET)

#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->ALG
#if defined (CONFIG_HNAT_V2)
#define FOE_ENTRY_VALID(skb)	    (((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FOE_Entry != 0x3fff)
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->SPORT
#else
#define FOE_ENTRY_VALID(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FVLD
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->SP	//src_port or user priority
#endif

#endif

// fast fill FoE desc field
#define DO_FILL_FOE_DESC(skb,desc)  (*(uint32_t *)(FOE_INFO_START_ADDR(skb)+2) = (uint32_t)(desc))

// fast fill FoE desc to DPORT PPE (magic_tag,entry_num)
#define DO_FILL_FOE_DPORT_PPE(skb)  (*(uint32_t *)(FOE_INFO_START_ADDR(skb)) = FOE_MAGIC_PPE_DWORD)

// fast clear FoE Info (magic_tag,entry_num)
#define DO_FAST_CLEAR_FOE(skb)	    (*(uint32_t *)(FOE_INFO_START_ADDR(skb)) = 0UL)

// full clear FoE Info
#define DO_FULL_CLEAR_FOE(skb)	    (memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN))

#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE))

#define IS_DPORT_PPE_VALID(skb)	    (*(uint32_t *)(FOE_INFO_START_ADDR(skb)) == FOE_MAGIC_PPE_DWORD)

#define FOE_ALG_MARK(skb)	    if (IS_SPACE_AVAILABLED(skb) && !FOE_ALG(skb) && IS_MAGIC_TAG_VALID(skb)) FOE_ALG(skb)=1
#define FOE_AI_UNHIT(skb)	    if (IS_SPACE_AVAILABLED(skb)) FOE_AI(skb)=UN_HIT

/*
 * EXPORT FUNCTION
 */
int PpeSetBindThreshold(uint32_t threshold);
int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt);
int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac,
		   uint16_t post_meter, uint16_t post_ac);

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka);
int PpeSetUnbindLifeTime(uint8_t lifetime);
int PpeSetBindLifetime(uint16_t tcp_fin, uint16_t udp_life, uint16_t fin_life);

struct FoeEntry *get_foe_entry(uint32_t foe_entry_num);

extern struct FoeEntry *get_foe_table(dma_addr_t *dma_handle, uint32_t *FoeTblSize);

#endif
