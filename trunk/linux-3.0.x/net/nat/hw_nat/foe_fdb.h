/*
    Module Name:
    foe_fdb.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef _FOE_FDB_WANTED
#define _FOE_FDB_WANTED

#include "hwnat_ioctl.h"
#include "frame_engine.h"
/*
 * DEFINITIONS AND MACROS
 */
#define FOE_ENTRY_LIFE_TIME	5
#define FOE_THRESHOLD		1000
#define FOE_HASH_MASK		0x00001FFF
#define FOE_HASH_WAY		2
#define FOE_1K_SIZ_MASK		0x000001FF
#define FOE_2K_SIZ_MASK		0x000003FF
#define FOE_4K_SIZ_MASK		0x000007FF
#define FOE_8K_SIZ_MASK		0x00000FFF
#define FOE_16K_SIZ_MASK	0x00001FFF

#if defined (CONFIG_RA_HW_NAT_TBL_1K)
#define FOE_4TB_SIZ		1024
#elif defined (CONFIG_RA_HW_NAT_TBL_2K)
#define FOE_4TB_SIZ		2048
#elif defined (CONFIG_RA_HW_NAT_TBL_4K)
#define FOE_4TB_SIZ		4096
#elif defined (CONFIG_RA_HW_NAT_TBL_8K)
#define FOE_4TB_SIZ		8192
#elif defined (CONFIG_RA_HW_NAT_TBL_16K)
#define FOE_4TB_SIZ		16384
#endif

#define FOE_ENTRY_SIZ		128	/* for ipv6 backward compatible */

#define IP_FORMAT(addr) \
	((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]

/*
 * TYPEDEFS AND STRUCTURES
 */

enum FoeTblSize {
	FoeTblSize_1K,
	FoeTblSize_2K,
	FoeTblSize_4K,
	FoeTblSize_8K,
	FoeTblSize_16K
};

enum VlanAction {
	NO_ACT = 0,
	MODIFY = 1,
	INSERT = 2,
	DELETE = 3
};

enum FoeEntryState {
	INVALID = 0,
	UNBIND = 1,
	BIND = 2,
	FIN = 3
};

enum FoeTblTcpUdp {
	TCP = 0,
	UDP = 1,
	ANY = 2
};

enum FoeTblEE {
	NOT_ENTRY_END = 0,
	ENTRY_END_FP = 1,
	ENTRY_END_FOE = 2
};

enum FoeLinkType {
	LINK_TO_FOE = 0,
	LINK_TO_FP = 1
};

enum FoeIPAct {
	IPV4_HNAPT = 0,
	IPV4_HNAT = 1,
	IPV6_1T_ROUTE = 2,
	IPV4_DSLITE = 3,
	IPV6_3T_ROUTE = 4,
	IPV6_5T_ROUTE = 5,
	IPV6_6RD = 7,
};

#define IS_IPV4_HNAPT(x)	(((x)->bfib1.pkt_type == IPV4_HNAPT) ? 1: 0)
#define IS_IPV4_HNAT(x)		(((x)->bfib1.pkt_type == IPV4_HNAT) ? 1 : 0)
#define IS_IPV6_1T_ROUTE(x)	(((x)->bfib1.pkt_type == IPV6_1T_ROUTE) ? 1 : 0)
#define IS_IPV4_DSLITE(x)	(((x)->bfib1.pkt_type == IPV4_DSLITE) ? 1 : 0)
#define IS_IPV6_3T_ROUTE(x)	(((x)->bfib1.pkt_type == IPV6_3T_ROUTE) ? 1 : 0)
#define IS_IPV6_5T_ROUTE(x)	(((x)->bfib1.pkt_type == IPV6_5T_ROUTE) ? 1 : 0)
#define IS_IPV6_6RD(x)		(((x)->bfib1.pkt_type == IPV6_6RD) ? 1: 0)
#define IS_IPV4_GRP(x)		(IS_IPV4_HNAPT(x) | IS_IPV4_HNAT(x))
#define IS_IPV6_GRP(x)		(IS_IPV6_1T_ROUTE(x) | IS_IPV6_3T_ROUTE(x) | IS_IPV6_5T_ROUTE(x) | IS_IPV6_6RD(x) |  IS_IPV4_DSLITE(x))

#if defined (CONFIG_HNAT_V2)

/* state = unbind & dynamic */
struct ud_info_blk1 {
	uint32_t time_stamp:8;
	uint32_t pcnt:16;	/* packet count */
	uint32_t resv:1;
	uint32_t pkt_type:3;
	uint32_t state:2;
	uint32_t udp:1;
	uint32_t sta:1;		/* static entry */
};

/* state = bind & fin */
struct bf_info_blk1 {
	uint32_t time_stamp:15;
	uint32_t ka:1;		/* keep alive */
	uint32_t vlan_layer:3;
	uint32_t psn:1;		/* egress packet has PPPoE session */
	uint32_t dvp:1;		/* inform switch of keeping VPRI */
	uint32_t drm:1;		/* inform switch of keeping DSCP(IPv4) or TC(IPv6) */
	uint32_t cah:1;		/* cacheable flag */
	uint32_t rmt:1;		/* remove tunnel ip header (6rd/dslite only) */
	uint32_t ttl:1;
	uint32_t pkt_type:3;
	uint32_t state:2;
	uint32_t udp:1;
	uint32_t sta:1;		/* static entry */
};

struct _info_blk2 {
	uint32_t fpidx:4;	/* force port index */
	uint32_t fp:1;		/* force new user priority */
	uint32_t up:3;		/* new user priority */
	uint32_t fdq:4;		/* force DRAM queue (for CAR case) */
	uint32_t port_mg:6;	/* port meter group */
	uint32_t port_ag:6;	/* port account group */
	uint32_t dscp:8;	/* DSCP value */
};

#else
/* state = unbind & dynamic */
struct ud_info_blk1 {
	uint32_t time_stamp:8;
	uint32_t pcnt:16;	/* packet count */
	uint32_t resv:2;
	uint32_t pkt_type:2;	/* entry format */
	uint32_t state:2;
	uint32_t udp:1;
	uint32_t sta:1;		/* static entry */
};

/* state = bind & fin */
struct bf_info_blk1 {
	uint16_t time_stamp;
	uint16_t v1:2;
	uint16_t v2:2;
	uint16_t snap:2;
	uint16_t pppoe:2;
	uint16_t ttl:1;
	uint16_t ka:1;		/* keep alive */
	uint16_t pkt_type:2;	/* entry format */
	uint16_t state:2;
	uint16_t udp:1;
	uint16_t sta:1;		/* static entry */
};

struct _info_blk2 {
	uint16_t fd:1;		/* force destination */
	uint16_t dp:3;		/* destination port (0:cpu,1:GE1) */
	uint16_t fp:1;		/* force new user priority */
	uint16_t up:3;		/* new user priority */
	uint16_t port_mg:6;	/* port meter group */
	uint16_t resv:1;
	uint16_t me:1;		/* meter enable */
	uint16_t port_ag:6;	/* port account group */
	uint16_t drm:1;		/* remark IPv4 DSCP */
	uint16_t ae:1;		/* account enable */
	uint16_t dscp:8;	/* DSCP value */
};
#endif

/*
 * Foe Entry (64B)
 *
 *      IPV4:			     IPV6:
 *	+-----------------------+    +-----------------------+
 *	|  Information Block 1  |    |  Information Block 1  |
 *	+-----------------------+    +-----------------------+
 *	|	SIP(4B)		|    |     IPv6_DIP0(4B)     |
 *	+-----------------------+    +-----------------------+
 *	|	DIP(4B)		|    |     IPv6_DIP1(4B)     |
 *	+-----------------------+    +-----------------------+
 *	| SPORT(2B) | DPORT(2B) |    |	      Rev(4B)        |
 *	+-----------+-----------+    +-----------------------+
 *	| Information Block 2   |    |	Information Block 2  | 
 *	+-----------------------+    +-----------------------+
 *	|      New SIP(4B)	|    |     IPv6_DIP2(4B)     |
 *	+-----------------------+    +-----------------------+
 *	|      New DIP(4B)	|    |     IPv6_DIP3(4B)     |
 *	+-----------------------+    +-----------------------+
 *	| New SPORT | New DPORT |    |	       Rev(4B)       |
 *	+-----------+-----------+    +-----------------------+
 *	| VLAN1(2B) |DMAC[47:32]|    | VLAN1(2B) |DMAC[47:32]|
 *	+-----------|-----------+    +-----------|-----------+
 *	|	DMAC[31:0]      |    |       DMAC[31:0]      |
 *	+-----------------------+    +-----------------------+
 *	| PPPoE_ID  |SMAC[47:32]|    | PPPoE_ID  |SMAC[47:32]|
 *	+-----------+-----------+    +-----------+-----------+
 *	|       SMAC[31:0]      |    |       SMAC[31:0]      | 
 *	+-----------------------+    +-----------------------+
 *	| Rev |  SNAP_Ctrl(3B)  |    | Rev |  SNAP_Ctrl(3B)  |
 *	+-----------------------+    +-----------------------+
 *	|    Rev    | VLAN2(2B) |    |   Rev     | VLAN2(2B) |
 *	+-----------------------+    +-----------------------+
 *	|     Rev(4B)           |    |       Rev(4B)         |
 *	+-----------------------+    +-----------------------+
 *	|     tmp_buf(4B)       |    |       tmp_buf(4B)     |
 *	+-----------------------+    +-----------------------+
 *
 * Foe Entry (80)
 *
 *      IPV4 HNAPT:			     IPV4:
 *	+-----------------------+    +-----------------------+
 *	|  Information Block 1  |    |  Information Block 1  |
 *	+-----------------------+    +-----------------------+
 *	|	SIP(4B)		|    |		SIP(4B)      |
 *	+-----------------------+    +-----------------------+
 *	|	DIP(4B)		|    |		DIP(4B)      |
 *	+-----------------------+    +-----------------------+
 *	| SPORT(2B) | DPORT(2B) |    |	      Rev(4B)        |
 *	+-----------+-----------+    +-----------------------+
 *	| EG DSCP| Info Block 2 |    |	Information Block 2  | 
 *	+-----------------------+    +-----------------------+
 *	|      New SIP(4B)	|    |     New SIP (4B)      |
 *	+-----------------------+    +-----------------------+
 *	|      New DIP(4B)	|    |     New DIP (4B)      |
 *	+-----------------------+    +-----------------------+
 *	| New SPORT | New DPORT |    | New SPORT | New DPORT |
 *	+-----------+-----------+    +-----------------------+
 *	|          REV          |    |		REV	     |
 *	+-----------------------+    +-----------------------+
 *	|Act_dp|   REV          |    |Act_dp|	REV	     |
 *	+-----------------------+    +-----------------------+
 *	|      tmp_buf(4B)      |    |	     temp_buf(4B)    | 
 *	+-----------------------+    +-----------|-----------+
 *	| ETYPE     | VLAN1 ID  |    | ETYPE     |  VLAN1    |
 *	+-----------+-----------+    +-----------+-----------+
 *	|       DMAC[47:16]     |    |       SMAC[47:16]     | 
 *	+-----------------------+    +-----------------------+
 *	| DMAC[15:0]| VLAN2 ID  |    | DMAC[15:0]|  VLAN2    |
 *	+-----------------------+    +-----------------------+
 *	|       SMAC[47:16]     |    |       SMAC[47:16]     | 
 *	+-----------------------+    +-----------------------+
 *	| SMAC[15:0]| PPPOE ID  |    | SMAC[15:0]| PPPOE ID  |
 *	+-----------------------+    +-----------------------+
 *
 */
#if defined (CONFIG_HNAT_V2)
struct _ipv4_hnapt {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t sip;
	uint32_t dip;
	uint16_t dport;
	uint16_t sport;
	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};
	uint32_t new_sip;
	uint32_t new_dip;
	uint16_t new_dport;
	uint16_t new_sport;
	uint32_t resv1;
	uint32_t resv2;
	uint32_t resv3:26;
	uint32_t act_dp:6;	/* UDF */
	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};

struct _ipv4_dslite {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t sip;
	uint32_t dip;
	uint16_t dport;
	uint16_t sport;

	uint32_t tunnel_sipv6_0;
	uint32_t tunnel_sipv6_1;
	uint32_t tunnel_sipv6_2;
	uint32_t tunnel_sipv6_3;

	uint32_t tunnel_dipv6_0;
	uint32_t tunnel_dipv6_1;
	uint32_t tunnel_dipv6_2;
	uint32_t tunnel_dipv6_3;

	uint8_t flow_lbl[3];	/* in order to consist with Linux kernel (should be 20bits) */
	uint16_t priority:4;	/* in order to consist with Linux kernel (should be 8bits) */
	uint16_t resv:4;
	uint32_t hop_limit:8;
	uint32_t resv2:18;
	uint32_t act_dp:6;	/* UDF */
	
	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};

	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};


struct _ipv6_1t_route {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t ipv6_dip0;
	uint32_t ipv6_dip1;
	uint32_t resv;

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};

	uint32_t ipv6_dip2;
	uint32_t ipv6_dip3;
	uint32_t resv1;

	uint32_t act_dp:6;	/* UDF */
	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};

struct _ipv6_3t_route {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t ipv6_sip0;
	uint32_t ipv6_sip1;
	uint32_t ipv6_sip2;
	uint32_t ipv6_sip3;
	uint32_t ipv6_dip0;
	uint32_t ipv6_dip1;
	uint32_t ipv6_dip2;
	uint32_t ipv6_dip3;
	uint32_t prot:8;
	uint32_t resv:24;
	
	uint32_t resv1;
	uint32_t resv2;
	uint32_t resv3;
	uint32_t resv4:26;
	uint32_t act_dp:6;	/* UDF */

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};
	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};
struct _ipv6_5t_route {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t ipv6_sip0;
	uint32_t ipv6_sip1;
	uint32_t ipv6_sip2;
	uint32_t ipv6_sip3;
	uint32_t ipv6_dip0;
	uint32_t ipv6_dip1;
	uint32_t ipv6_dip2;
	uint32_t ipv6_dip3;
	uint16_t dport;
	uint16_t sport;
	
	uint32_t resv1;
	uint32_t resv2;
	uint32_t resv3;
	uint32_t resv4:26;
	uint32_t act_dp:6;	/* UDF */

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};
	
	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};

struct _ipv6_6rd {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t ipv6_sip0;
	uint32_t ipv6_sip1;
	uint32_t ipv6_sip2;
	uint32_t ipv6_sip3;
	uint32_t ipv6_dip0;
	uint32_t ipv6_dip1;
	uint32_t ipv6_dip2;
	uint32_t ipv6_dip3;
	uint16_t dport;
	uint16_t sport;

	uint32_t tunnel_sipv4;
	uint32_t tunnel_dipv4;
	uint32_t hdr_chksum:16;
	uint32_t dscp:8;
	uint32_t ttl:8;
	uint32_t flag:3;
	uint32_t resv1:23;
	uint32_t act_dp:6;	/* UDF */

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};

	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];


};

struct FoeEntry {
	union {
		struct bf_info_blk1 bfib1;	//common header
		struct _ipv4_hnapt ipv4_hnapt;	// nat & napt share same data structure
#if defined (CONFIG_RA_HW_NAT_IPV6)
		struct _ipv4_dslite ipv4_dslite;
		struct _ipv6_1t_route ipv6_1t_route;
		struct _ipv6_3t_route ipv6_3t_route;
		struct _ipv6_5t_route ipv6_5t_route;
		struct _ipv6_6rd ipv6_6rd;
#endif
	};
};

struct FoePriKey {
	/* TODO: add new primary key to support dslite, 6rd */

	/* Ipv4 */
	struct {
		uint32_t sip;
		uint32_t dip;
		uint16_t sport;
		uint16_t dport;
		uint32_t is_udp:1;
	} ipv4_hnapt;

	struct {
		uint32_t sip;
		uint32_t dip;
		/* TODO */
	} ipv4_hnat;

	struct {
		uint32_t sip;
		uint32_t dip;
		/* TODO */
	} ipv4_dslite;

	/* IPv6 */
	struct {
		uint32_t dip0;
		uint32_t dip1;
		uint32_t dip2;
		uint32_t dip3;
	} ipv6_routing;

	struct {
		/* TODO */
	} ipv6_6rd;

	uint32_t pkt_type;	/* entry format */
};

#else
struct _ipv4_hnapt {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t sip;
	uint32_t dip;
	uint16_t dport;
	uint16_t sport;

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};

	uint32_t new_sip;
	uint32_t new_dip;
	uint16_t new_dport;
	uint16_t new_sport;

	uint8_t dmac_hi[2];
	uint16_t vlan1;
	uint8_t dmac_lo[4];
	uint8_t smac_hi[2];
	uint16_t pppoe_id;
	uint8_t smac_lo[4];

	uint8_t snap_ctrl[3];
	uint8_t act_dp:6;	/* UDF */
	uint8_t resv1:2;
	uint16_t vlan2;
	uint16_t resv2;
	uint32_t resv3;
	uint32_t resv4;
};

struct _ipv6_1t_route {
	union {
		struct ud_info_blk1 udib1;
		struct bf_info_blk1 bfib1;
		uint32_t info_blk1;
	};
	uint32_t ipv6_dip0;
	uint32_t ipv6_dip1;
	uint32_t resv;

	union {
		struct _info_blk2 iblk2;
		uint32_t info_blk2;
	};

	uint32_t ipv6_dip2;
	uint32_t ipv6_dip3;
	uint32_t resv1;

	uint32_t act_dp:6;	/* UDF */
	uint16_t vlan1;
	uint16_t etype;
	uint8_t dmac_hi[4];
	uint16_t vlan2;
	uint8_t dmac_lo[2];
	uint8_t smac_hi[4];
	uint16_t pppoe_id;
	uint8_t smac_lo[2];
};

struct FoeEntry {
	union {
		struct bf_info_blk1 bfib1;	//common header
		struct _ipv4_hnapt ipv4_hnapt;	// nat & napt share same data structure
		struct _ipv6_1t_route ipv6_1t_route;
	};
};

struct FoePriKey {
	/* Ipv4 */
	struct {
		uint32_t sip;
		uint32_t dip;
		uint16_t sport;
		uint16_t dport;
		uint32_t is_udp:1;
	} ipv4;

	/* IPv6 */
	struct {
		uint32_t dip0;
		uint32_t dip1;
		uint32_t dip2;
		uint32_t dip3;
	} ipv6;

	uint32_t pkt_type;	/* entry format */
};
#endif

int32_t FoeHashFun(struct FoePriKey *key, enum FoeEntryState TargetState);
uint32_t FoeInsEntry(struct FoeEntry *entry);
void FoeSetMacInfo(uint8_t * dst, uint8_t * src);
void FoeGetMacInfo(uint8_t * dst, uint8_t * src);
void FoeDumpEntry(uint32_t Index);
int FoeGetAllEntries(struct hwnat_args *opt);
int FoeBindEntry(struct hwnat_args *opt);
int FoeUnBindEntry(struct hwnat_args *opt);
int FoeDelEntryByNum(uint32_t entry_num);
void FoeTblClean(void);
int FoeDumpCacheEntry(void);

#endif
