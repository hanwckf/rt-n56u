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
#include "foe_fdb.h"
#include "hwnat_ioctl.h"
#include "util.h"

extern struct FoeEntry *PpeFoeBase;
extern uint32_t PpeFoeTblSize;

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
void FoeSetMacInfo(uint8_t * Dst, uint8_t * Src)
{
	Dst[1] = Src[0];
	Dst[0] = Src[1];
	Dst[7] = Src[2];
	Dst[6] = Src[3];
	Dst[5] = Src[4];
	Dst[4] = Src[5];
}

void FoeGetMacInfo(uint8_t * Dst, uint8_t * Src)
{
	Dst[0] = Src[1];
	Dst[1] = Src[0];
	Dst[2] = Src[7];
	Dst[3] = Src[6];
	Dst[4] = Src[5];
	Dst[5] = Src[4];
}

void FoeDumpEntry(uint32_t Index)
{
	struct FoeEntry *entry = &PpeFoeBase[Index];
	uint32_t *p = (uint32_t *)entry;
	uint32_t i = 0;

	NAT_PRINT("==========<Flow Table Entry=%d (%p)>===============\n", Index, entry);
	for(i=0; i < 16; i++) { // 64 bytes per entry
		printk("%02d: %08X\n", i,*(p+i));
	}
	NAT_PRINT("-----------------<Flow Info>------------------\n");
	NAT_PRINT("Information Block 1: %08X\n", entry->ipv4_hnapt.info_blk1);

	if (IS_IPV4_HNAPT(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv4_hnapt.info_blk2);
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
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv4_hnapt.info_blk2);
		NAT_PRINT("Create IPv4 HNAT entry\n");
		NAT_PRINT("IPv4 Org IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.sip), IP_FORMAT(entry->ipv4_hnapt.dip));
		NAT_PRINT("IPv4 New IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.new_sip), IP_FORMAT(entry->ipv4_hnapt.new_dip));
#if defined (CONFIG_RA_HW_NAT_IPV6)
	} else if (IS_IPV6_1T_ROUTE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv6_1t_route.info_blk2);
		NAT_PRINT("Create IPv6 Route entry\n");
		NAT_PRINT("Destination IPv6: %08X:%08X:%08X:%08X",
			  entry->ipv6_1t_route.ipv6_dip3, entry->ipv6_1t_route.ipv6_dip2,
			  entry->ipv6_1t_route.ipv6_dip1, entry->ipv6_1t_route.ipv6_dip0);
#endif
	}

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
}

int FoeGetAllEntries(struct hwnat_args *opt)
{
	struct FoeEntry *entry;
	int hash_index = 0;
	int count = 0;		/* valid entry count */

	for (hash_index = 0; hash_index < PpeFoeTblSize; hash_index++) {
		entry = &PpeFoeBase[hash_index];

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
			} else if (IS_IPV6_1T_ROUTE(entry)) {
				opt->entries[count].ing_dipv6_0 = entry->ipv6_1t_route.ipv6_dip3;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_1t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_1t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_1t_route.ipv6_dip0;
				count++;
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

	entry = &PpeFoeBase[opt->entry_num];

	//restore right information block1
	entry->bfib1.time_stamp = RegRead(FOE_TS) & 0xFFFF;
	entry->bfib1.state = BIND;

	return HWNAT_SUCCESS;
}

int FoeUnBindEntry(struct hwnat_args *opt)
{

	struct FoeEntry *entry;

	entry = &PpeFoeBase[opt->entry_num];

	entry->ipv4_hnapt.udib1.state = UNBIND;
	entry->ipv4_hnapt.udib1.time_stamp = RegRead(FOE_TS) & 0xFF;

	return HWNAT_SUCCESS;
}

int FoeDelEntryByNum(uint32_t entry_num)
{
	struct FoeEntry *entry;

	entry = &PpeFoeBase[entry_num];
	memset(entry, 0, sizeof(struct FoeEntry));

	return HWNAT_SUCCESS;
}


