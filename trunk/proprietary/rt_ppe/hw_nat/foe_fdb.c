/*
    Module Name:
    foe_fdb.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "util.h"
#include "ra_nat.h"

#if defined (CONFIG_RA_HW_NAT_IPV6)
extern int ipv6_offload;
#endif

extern uint32_t PpeFoeTblSize;
extern spinlock_t ppe_foe_lock;

#if defined (CONFIG_HNAT_V2)
/*
 * 4          2         0
 * +----------+---------+
 * |      DMAC[47:16]   |
 * +--------------------+
 * |DMAC[15:0]| 2nd VID |
 * +----------+---------+
 * 
 * 4          2         0
 * +----------+---------+
 * |      SMAC[47:16]   |
 * +--------------------+
 * |SMAC[15:0]| PPPOE ID|
 * +----------+---------+
 * 
 * Ex: 
 *
 * Mac=01:22:33:44:55:66
 *
 * 4          2         0
 * +----------+---------+
 * |     01:22:33:44    |
 * +--------------------+
 * |  55:66   | PPPOE ID|
 * +----------+---------+
 */

void FoeSetMacHiInfo(uint8_t *Dst, uint8_t *Src)
{
	Dst[3] = Src[0];
	Dst[2] = Src[1];
	Dst[1] = Src[2];
	Dst[0] = Src[3];
}

void FoeSetMacLoInfo(uint8_t *Dst, uint8_t *Src)
{
	Dst[1] = Src[4];
	Dst[0] = Src[5];
}

#else
/* 
 * Mac address is not continuous in foe table
 *
 * 4      2	  0
 * +------+-------+
 * |VLAN  |DMac_hi|
 * +------+-------+
 * |  Dmac_lo     |
 * +--------------+
 *
 * 4      2	  0
 * +------+-------+
 * |PPPoE |SMac_hi|
 * +------+-------+
 * |  Smac_lo     |
 * +--------------+
 *
 * Ex: 
 *
 * Mac=01:80:C2:01:23:45
 *
 * 4      2	  0
 * +------+-------+
 * |PPPoE | 01:80 |
 * +------+-------+
 * | C2:01:23:45  |
 * +--------------+
 *
 */

void FoeSetMacHiInfo(uint8_t *Dst, uint8_t *Src)
{
	Dst[1] = Src[0];
	Dst[0] = Src[1];
}

void FoeSetMacLoInfo(uint8_t *Dst, uint8_t *Src)
{
	Dst[3] = Src[2];
	Dst[2] = Src[3];
	Dst[1] = Src[4];
	Dst[0] = Src[5];
}

#endif

#if defined (CONFIG_HNAT_V2)
static int is_request_done(void)
{
	int count = 1000;

	//waiting for 1sec to make sure action was finished
	do {
		if (((RegRead(CAH_CTRL) >> 8) & 0x1) == 0) {
			return 1;
		}

		udelay(1000);
	} while(--count);

	return 0;
}

#define MAX_CACHE_LINE_NUM		16
int FoeDumpCacheEntry(void)
{
	int line = 0;
	int state = 0;
	int tag = 0;
	int cah_en = 0;
	int i = 0;

	cah_en = RegRead(CAH_CTRL) & 0x1;

	if(!cah_en) {
		NAT_PRINT("Cache is not enabled\n");
		return 0;
	}

	// cache disable
	RegModifyBits(CAH_CTRL, 0, 0, 1);

	NAT_PRINT(" No  |   State   |   Tag        \n");
	NAT_PRINT("-----+-----------+------------  \n");
	for(line = 0; line < MAX_CACHE_LINE_NUM; line++) {
	
		//set line number
		RegModifyBits(CAH_LINE_RW, line, 0, 15);

		//OFFSET_RW = 0x1F (Get Entry Number)
		RegModifyBits(CAH_LINE_RW, 0x1F, 16, 8);

		//software access cache command = read	
		RegModifyBits(CAH_CTRL, 2, 12, 2);

		//trigger software access cache request
		RegModifyBits(CAH_CTRL, 1, 8, 1);

		if (is_request_done()) {
			tag = (RegRead(CAH_RDATA) & 0xFFFF) ;
			state = ((RegRead(CAH_RDATA)>>16) & 0x3) ;
			NAT_PRINT("%04d | %s   | %05d\n", line, 
					(state==3) ? " Lock  " : 
					(state==2) ? " Dirty " :
					(state==1) ? " Valid " : 
						     "Invalid", tag);
		} else {
			NAT_PRINT("%s is timeout (%d)\n", __FUNCTION__, line);
		}
		
		//software access cache command = read	
		RegModifyBits(CAH_CTRL, 3, 12, 2);

		RegWrite(CAH_WDATA,0);

		//trigger software access cache request
		RegModifyBits(CAH_CTRL, 1, 8, 1);

		if (!is_request_done()) {
			NAT_PRINT("%s is timeout (%d)\n", __FUNCTION__, line);
		}

		/* dump first 16B for each foe entry */
		NAT_PRINT("==========<Flow Table Entry=%d >===============\n", tag);
		for(i = 0; i< 16; i++ ) {
			RegModifyBits(CAH_LINE_RW, i, 16, 8);
			
			//software access cache command = read	
			RegModifyBits(CAH_CTRL, 2, 12, 2);

			//trigger software access cache request
			RegModifyBits(CAH_CTRL, 1, 8, 1);

			if (is_request_done()) {
				NAT_PRINT("%02d  %08X\n", i, RegRead(CAH_RDATA));
			} else {
				NAT_PRINT("%s is timeout (%d)\n", __FUNCTION__, line);
			}
			
			//software access cache command = write	
			RegModifyBits(CAH_CTRL, 3, 12, 2);
			
			RegWrite(CAH_WDATA, 0);

			//trigger software access cache request
			RegModifyBits(CAH_CTRL, 1, 8, 1);

			if (!is_request_done()) {
				NAT_PRINT("%s is timeout (%d)\n", __FUNCTION__, line);
			}
		}
		NAT_PRINT("=========================================\n");
	}

	//clear cache table before enabling cache
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);

	// cache enable
	RegModifyBits(CAH_CTRL, 1, 0, 1);

	return 1;
}
#endif

static void FoePrintInfoBlk1(uint32_t info_blk1)
{
	NAT_PRINT("Information Block 1: %08X\n", info_blk1);
}

static void FoePrintInfoBlk2(uint32_t info_blk2)
{
	struct _info_blk2 *iblk2 = (struct _info_blk2 *)&info_blk2;

#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RALINK_MT7621)
	NAT_PRINT("Information Block 2: %08X (DP=%d, FQOS=%d, QID=%d, MCAST=%d)\n",
			info_blk2,
			iblk2->dp,
			iblk2->fqos,
			iblk2->qid,
			iblk2->mcast);
#else
	NAT_PRINT("Information Block 2: %08X (FPORT=%d)\n",
			info_blk2,
			iblk2->fpidx);
#endif
#else
	NAT_PRINT("Information Block 2: %08X (DP=%d, FORCE=%d)\n",
			info_blk2,
			iblk2->dp,
			iblk2->fd);
#endif
}

void FoeDumpEntry(uint32_t Index)
{
	struct FoeEntry *entry;
	uint32_t *p;
	uint32_t i, max_bytes;

	if (Index >= FOE_4TB_SIZ) {
		NAT_PRINT("Entry number (%d) is invalid!\n", Index);
		return;
	}

	entry = get_foe_entry(Index);
	p = (uint32_t *)entry;

	max_bytes = 16; // 64 bytes per entry
#if defined (CONFIG_HNAT_V2) && defined (CONFIG_RA_HW_NAT_IPV6)
	if (ipv6_offload)
		max_bytes = 20; // 80 bytes per entry
#endif

	NAT_PRINT("==========<Flow Table Entry=%d (%p)>===============\n", Index, entry);
	for(i=0; i < max_bytes; i++) {
		NAT_PRINT("%02d: %08X\n", i, *(p+i));
	}
	NAT_PRINT("-----------------<Flow Info>------------------\n");
	FoePrintInfoBlk1(entry->ipv4_hnapt.info_blk1);

	if (IS_IPV4_HNAPT(entry)) {
		FoePrintInfoBlk2(entry->ipv4_hnapt.info_blk2);
		NAT_PRINT("Create IPv4 HNAPT entry\n");
		NAT_PRINT
		    ("IPv4 Org IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		     IP_FORMAT(entry->ipv4_hnapt.sip), entry->ipv4_hnapt.sport,
		     IP_FORMAT(entry->ipv4_hnapt.dip), entry->ipv4_hnapt.dport);
		NAT_PRINT
		    ("IPv4 New IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		     IP_FORMAT(entry->ipv4_hnapt.new_sip), entry->ipv4_hnapt.new_sport,
		     IP_FORMAT(entry->ipv4_hnapt.new_dip), entry->ipv4_hnapt.new_dport);
	} else if (IS_IPV4_HNAT(entry)) {
		FoePrintInfoBlk2(entry->ipv4_hnapt.info_blk2);
		NAT_PRINT("Create IPv4 HNAT entry\n");
		NAT_PRINT("IPv4 Org IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.sip), IP_FORMAT(entry->ipv4_hnapt.dip));
		NAT_PRINT("IPv4 New IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.new_sip), IP_FORMAT(entry->ipv4_hnapt.new_dip));
#if defined (CONFIG_RA_HW_NAT_IPV6)
#if !defined (CONFIG_HNAT_V2)
	} else if (IS_IPV6_1T_ROUTE(entry)) {
		FoePrintInfoBlk2(entry->ipv6_1t_route.info_blk2);
		NAT_PRINT("Create IPv6 Route entry\n");
		NAT_PRINT("Destination IPv6: %08X:%08X:%08X:%08X",
			  entry->ipv6_1t_route.ipv6_dip3, entry->ipv6_1t_route.ipv6_dip2,
			  entry->ipv6_1t_route.ipv6_dip1, entry->ipv6_1t_route.ipv6_dip0);
#else
	} else if (IS_IPV4_DSLITE(entry)) {
		FoePrintInfoBlk2(entry->ipv4_dslite.info_blk2);
		NAT_PRINT("Create IPv4 Ds-Lite entry\n");
		NAT_PRINT
		    ("IPv4 Ds-Lite: %u.%u.%u.%u.%d->%u.%u.%u.%u:%d\n ",
		     IP_FORMAT(entry->ipv4_dslite.sip), entry->ipv4_dslite.sport,
		     IP_FORMAT(entry->ipv4_dslite.dip), entry->ipv4_dslite.dport);
		NAT_PRINT("EG DIPv6: %08X:%08X:%08X:%08X->%08X:%08X:%08X:%08X\n",
			  entry->ipv4_dslite.tunnel_sipv6_0, entry->ipv4_dslite.tunnel_sipv6_1,
			  entry->ipv4_dslite.tunnel_sipv6_2, entry->ipv4_dslite.tunnel_sipv6_3,
			  entry->ipv4_dslite.tunnel_dipv6_0, entry->ipv4_dslite.tunnel_dipv6_1,
			  entry->ipv4_dslite.tunnel_dipv6_2, entry->ipv4_dslite.tunnel_dipv6_3);
	} else if (IS_IPV6_3T_ROUTE(entry)) {
		FoePrintInfoBlk2(entry->ipv6_3t_route.info_blk2);
		NAT_PRINT("Create IPv6 3-Tuple entry\n");
		NAT_PRINT
		    ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Prot=%d)\n",
		     entry->ipv6_3t_route.ipv6_sip0, entry->ipv6_3t_route.ipv6_sip1,
		     entry->ipv6_3t_route.ipv6_sip2, entry->ipv6_3t_route.ipv6_sip3,
		     entry->ipv6_3t_route.ipv6_dip0, entry->ipv6_3t_route.ipv6_dip1,
		     entry->ipv6_3t_route.ipv6_dip2, entry->ipv6_3t_route.ipv6_dip3, 
		     entry->ipv6_3t_route.prot);
	} else if (IS_IPV6_5T_ROUTE(entry)) {
		FoePrintInfoBlk2(entry->ipv6_5t_route.info_blk2);
		NAT_PRINT("Create IPv6 5-Tuple entry\n");
		if(IS_IPV6_FLAB_EBL()) {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Flow Label=%08X) \n",
				entry->ipv6_5t_route.ipv6_sip0, entry->ipv6_5t_route.ipv6_sip1,
				entry->ipv6_5t_route.ipv6_sip2, entry->ipv6_5t_route.ipv6_sip3,
				entry->ipv6_5t_route.ipv6_dip0, entry->ipv6_5t_route.ipv6_dip1,
				entry->ipv6_5t_route.ipv6_dip2, entry->ipv6_5t_route.ipv6_dip3,
				((entry->ipv6_5t_route.sport << 16) | (entry->ipv6_5t_route.dport))&0xFFFFF);
		} else {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d \n",
				entry->ipv6_5t_route.ipv6_sip0, entry->ipv6_5t_route.ipv6_sip1,
				entry->ipv6_5t_route.ipv6_sip2, entry->ipv6_5t_route.ipv6_sip3,
				entry->ipv6_5t_route.sport,
				entry->ipv6_5t_route.ipv6_dip0, entry->ipv6_5t_route.ipv6_dip1,
				entry->ipv6_5t_route.ipv6_dip2, entry->ipv6_5t_route.ipv6_dip3,
				entry->ipv6_5t_route.dport);
		}
	} else if (IS_IPV6_6RD(entry)) {
		FoePrintInfoBlk2(entry->ipv6_6rd.info_blk2);
		NAT_PRINT("Create IPv6 6RD entry\n");
		if(IS_IPV6_FLAB_EBL()) {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Flow Label=%08X) \n",
				entry->ipv6_6rd.ipv6_sip0, entry->ipv6_6rd.ipv6_sip1,
				entry->ipv6_6rd.ipv6_sip2, entry->ipv6_6rd.ipv6_sip3,
				entry->ipv6_6rd.ipv6_dip0, entry->ipv6_6rd.ipv6_dip1, 
				entry->ipv6_6rd.ipv6_dip2, entry->ipv6_6rd.ipv6_dip3, 
				((entry->ipv6_5t_route.sport << 16) | (entry->ipv6_5t_route.dport))&0xFFFFF);
		} else {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d \n",
				entry->ipv6_6rd.ipv6_sip0, entry->ipv6_6rd.ipv6_sip1,
				entry->ipv6_6rd.ipv6_sip2, entry->ipv6_6rd.ipv6_sip3,
				entry->ipv6_6rd.sport, 
				entry->ipv6_6rd.ipv6_dip0, entry->ipv6_6rd.ipv6_dip1, 
				entry->ipv6_6rd.ipv6_dip2, entry->ipv6_6rd.ipv6_dip3, 
				entry->ipv6_6rd.dport);
		}
#endif
#endif
	}

#if defined (CONFIG_HNAT_V2)
	if (IS_IPV4_HNAPT(entry) || IS_IPV4_HNAT(entry)) {
	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv4_hnapt.dmac_hi[3], entry->ipv4_hnapt.dmac_hi[2],
	     entry->ipv4_hnapt.dmac_hi[1], entry->ipv4_hnapt.dmac_hi[0],
	     entry->ipv4_hnapt.dmac_lo[1], entry->ipv4_hnapt.dmac_lo[0],
	     entry->ipv4_hnapt.smac_hi[3], entry->ipv4_hnapt.smac_hi[2],
	     entry->ipv4_hnapt.smac_hi[1], entry->ipv4_hnapt.smac_hi[0],
	     entry->ipv4_hnapt.smac_lo[1], entry->ipv4_hnapt.smac_lo[0]);
	    NAT_PRINT("=========================================\n\n");
	} 
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (ipv6_offload) {
	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv6_5t_route.dmac_hi[3], entry->ipv6_5t_route.dmac_hi[2],
	     entry->ipv6_5t_route.dmac_hi[1], entry->ipv6_5t_route.dmac_hi[0],
	     entry->ipv6_5t_route.dmac_lo[1], entry->ipv6_5t_route.dmac_lo[0],
	     entry->ipv6_5t_route.smac_hi[3], entry->ipv6_5t_route.smac_hi[2],
	     entry->ipv6_5t_route.smac_hi[1], entry->ipv6_5t_route.smac_hi[0],
	     entry->ipv6_5t_route.smac_lo[1], entry->ipv6_5t_route.smac_lo[0]);
	    NAT_PRINT("=========================================\n\n");
	}
#endif

#else
	if (IS_IPV4_HNAPT(entry) || IS_IPV4_HNAT(entry) || IS_IPV6_1T_ROUTE(entry)) {

	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv4_hnapt.dmac_hi[1], entry->ipv4_hnapt.dmac_hi[0],
	     entry->ipv4_hnapt.dmac_lo[3], entry->ipv4_hnapt.dmac_lo[2],
	     entry->ipv4_hnapt.dmac_lo[1], entry->ipv4_hnapt.dmac_lo[0],
	     entry->ipv4_hnapt.smac_hi[1], entry->ipv4_hnapt.smac_hi[0],
	     entry->ipv4_hnapt.smac_lo[3], entry->ipv4_hnapt.smac_lo[2],
	     entry->ipv4_hnapt.smac_lo[1], entry->ipv4_hnapt.smac_lo[0]);
	    NAT_PRINT("=========================================\n\n");
	}
#endif
}

int FoeGetAllEntries(struct hwnat_args *opt)
{
	struct FoeEntry *entry;
	int hash_index = 0;
	int count = 0;		/* valid entry count */

	for (hash_index = 0; hash_index < PpeFoeTblSize; hash_index++) {
		entry = get_foe_entry(hash_index);

		if (entry->bfib1.state == opt->entry_state) {
			opt->entries[count].hash_index = hash_index;
			opt->entries[count].pkt_type =
			    entry->ipv4_hnapt.bfib1.pkt_type;

			if (IS_IPV4_HNAT(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_hnapt.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_hnapt.dip;
				opt->entries[count].eg_sipv4 = entry->ipv4_hnapt.new_sip;
				opt->entries[count].eg_dipv4 = entry->ipv4_hnapt.new_dip;
				count++;
			} else if (IS_IPV4_HNAPT(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_hnapt.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_hnapt.dip;
				opt->entries[count].eg_sipv4 = entry->ipv4_hnapt.new_sip;
				opt->entries[count].eg_dipv4 = entry->ipv4_hnapt.new_dip;
				opt->entries[count].ing_sp = entry->ipv4_hnapt.sport;
				opt->entries[count].ing_dp = entry->ipv4_hnapt.dport;
				opt->entries[count].eg_sp = entry->ipv4_hnapt.new_sport;
				opt->entries[count].eg_dp = entry->ipv4_hnapt.new_dport;
				count++;
#if defined (CONFIG_RA_HW_NAT_IPV6)
#if !defined (CONFIG_HNAT_V2)
			} else if (IS_IPV6_1T_ROUTE(entry)) {
				opt->entries[count].ing_dipv6_0 = entry->ipv6_1t_route.ipv6_dip3;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_1t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_1t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_1t_route.ipv6_dip0;
				count++;
#else
			} else if (IS_IPV4_DSLITE(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_dslite.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_dslite.dip;
				opt->entries[count].ing_sp = entry->ipv4_dslite.sport;
				opt->entries[count].ing_dp = entry->ipv4_dslite.dport;
				opt->entries[count].eg_sipv6_0 = entry->ipv4_dslite.tunnel_sipv6_0;
				opt->entries[count].eg_sipv6_1 = entry->ipv4_dslite.tunnel_sipv6_1;
				opt->entries[count].eg_sipv6_2 = entry->ipv4_dslite.tunnel_sipv6_2;
				opt->entries[count].eg_sipv6_3 = entry->ipv4_dslite.tunnel_sipv6_3;
				opt->entries[count].eg_dipv6_0 = entry->ipv4_dslite.tunnel_dipv6_0;
				opt->entries[count].eg_dipv6_1 = entry->ipv4_dslite.tunnel_dipv6_1;
				opt->entries[count].eg_dipv6_2 = entry->ipv4_dslite.tunnel_dipv6_2;
				opt->entries[count].eg_dipv6_3 = entry->ipv4_dslite.tunnel_dipv6_3;
				count++;
			} else if (IS_IPV6_3T_ROUTE(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_3t_route.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_3t_route.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_3t_route.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_3t_route.ipv6_sip3;
				opt->entries[count].ing_dipv6_0 = entry->ipv6_3t_route.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_3t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_3t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_3t_route.ipv6_dip3;
				opt->entries[count].prot = entry->ipv6_3t_route.prot;
				count++;
			} else if (IS_IPV6_5T_ROUTE(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_5t_route.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_5t_route.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_5t_route.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_5t_route.ipv6_sip3;
				opt->entries[count].ing_sp = entry->ipv6_5t_route.sport;
				opt->entries[count].ing_dp = entry->ipv6_5t_route.dport;

				opt->entries[count].ing_dipv6_0 = entry->ipv6_5t_route.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_5t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_5t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_5t_route.ipv6_dip3;
				opt->entries[count].ipv6_flowlabel = IS_IPV6_FLAB_EBL();
				count++;
			} else if (IS_IPV6_6RD(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_6rd.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_6rd.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_6rd.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_6rd.ipv6_sip3;

				opt->entries[count].ing_dipv6_0 = entry->ipv6_6rd.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_6rd.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_6rd.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_6rd.ipv6_dip3;
				opt->entries[count].ing_sp = entry->ipv6_6rd.sport;
				opt->entries[count].ing_dp = entry->ipv6_6rd.dport;
				opt->entries[count].ipv6_flowlabel = IS_IPV6_FLAB_EBL();

				opt->entries[count].eg_sipv4 = entry->ipv6_6rd.tunnel_sipv4;
				opt->entries[count].eg_dipv4 = entry->ipv6_6rd.tunnel_dipv4;
				count++;
#endif
#endif
			}
		}
	}

	opt->num_of_entries = count;

	if (opt->num_of_entries > 0) {
		return HWNAT_SUCCESS;
	} else {
		return HWNAT_ENTRY_NOT_FOUND;
	}
}

int FoeBindEntry(struct hwnat_args *opt)
{
	struct FoeEntry *entry;

	if (opt->entry_num >= FOE_4TB_SIZ)
		return HWNAT_ENTRY_NOT_FOUND;

	entry = get_foe_entry(opt->entry_num);

	//restore right information block1
	spin_lock_bh(&ppe_foe_lock);
	entry->bfib1.time_stamp = RegRead(FOE_TS) & 0xFFFF;
	entry->bfib1.state = BIND;
	spin_unlock_bh(&ppe_foe_lock);

	return HWNAT_SUCCESS;
}

int FoeUnBindEntry(struct hwnat_args *opt)
{
	struct FoeEntry *entry;

	if (opt->entry_num >= FOE_4TB_SIZ)
		return HWNAT_ENTRY_NOT_FOUND;

	entry = get_foe_entry(opt->entry_num);

	spin_lock_bh(&ppe_foe_lock);
	entry->udib1.state = INVALID;
	entry->udib1.time_stamp = RegRead(FOE_TS) & 0xFF;

#if defined (CONFIG_HNAT_V2)
	/* clear HWNAT cache */
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);
#endif
	spin_unlock_bh(&ppe_foe_lock);

	return HWNAT_SUCCESS;
}

#if defined (CONFIG_HNAT_V2)
int FoeDropEntry(struct hwnat_args *opt)
{
#if defined (CONFIG_RALINK_MT7621)
	struct FoeEntry *entry;

	if (opt->entry_num >= FOE_4TB_SIZ)
		return HWNAT_ENTRY_NOT_FOUND;

	entry = get_foe_entry(opt->entry_num);

	spin_lock_bh(&ppe_foe_lock);
	entry->ipv4_hnapt.iblk2.dp = 7;

	/* clear HWNAT cache */
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);
	spin_unlock_bh(&ppe_foe_lock);

	return HWNAT_SUCCESS;
#else
	return HWNAT_FAIL;
#endif
}
#endif

int FoeDelEntry(struct hwnat_args *opt)
{
	struct FoeEntry *entry;

	if (opt->entry_num >= FOE_4TB_SIZ)
		return HWNAT_ENTRY_NOT_FOUND;

	entry = get_foe_entry(opt->entry_num);

	spin_lock_bh(&ppe_foe_lock);
	memset(entry, 0, sizeof(struct FoeEntry));
	spin_unlock_bh(&ppe_foe_lock);

	return HWNAT_SUCCESS;
}

