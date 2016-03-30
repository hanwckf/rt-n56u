/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "../ra_esw_reg.h"
#include "../ra_esw_base.h"
#include "ioctl.h"
#if defined (CONFIG_MT7530_GSW)
#include "ioctl_mt762x.h"
#include "../ra_gsw_mt7530.h"
#elif defined (CONFIG_RALINK_MT7620)
#include "ioctl_mt762x.h"
#include "../ra_esw_mt7620.h"
#else
#include "ioctl_rt305x.h"
#include "../ra_esw_rt305x.h"
#endif

//#define IGMP_SN_DBG

#define MCAST_TABLE_MAX_SIZE		128
#define MCAST_TABLE_HASH_SIZE		64	// must be power of 2 and <= 256
#define MCAST_GROUP_CHECK_INERVAL	 (33 * HZ)
#define MCAST_GROUP_MEMBERSHIP_EXPIRED	(260 * HZ) // (125 * 2) + 10

#define MCAST_ADDR_HASH(addr)		(addr[0]^addr[1]^addr[2]^addr[3]^addr[4]^addr[5])
#define MCAST_ADDR_HASH_INDEX(addr)	(MCAST_ADDR_HASH(addr) & (MCAST_TABLE_HASH_SIZE - 1))

#define MAC_LOOKUP_RETRY_MAX		20

struct mcast_member_entry
{
	struct mcast_member_entry*	next;
	u16				entered;
	u8				haddr[ETH_ALEN];
};

struct mcast_group_entry
{
	struct mcast_group_entry*	next;
	struct mcast_member_entry*	members[ESW_EPHY_ID_MAX+1];
	unsigned long			last_time;
	u16				valid:1;
	u16				unused:3;
	u16				cvid:12;
	u8				maddr[ETH_ALEN];
};

struct mcast_table
{
	struct mcast_group_entry*	hash[MCAST_TABLE_HASH_SIZE];
	struct mcast_group_entry	pool[MCAST_TABLE_MAX_SIZE];
};

struct mcast_work
{
	struct work_struct ws;
	u8 mac_src[ETH_ALEN];
	u8 mac_dst[ETH_ALEN];
	u16 is_leave:2;
	u16 unused:2;
	u16 port_cvid:12;
};

static DEFINE_MUTEX(g_lut_lock);
static DEFINE_MUTEX(g_mtb_lock);
static struct mcast_table g_mtb;
static struct timer_list g_membership_expired_timer;
static u32 g_igmp_sn_enabled = 0;
static u32 g_igmp_sn_static_ports = 0;

extern void (*br_mcast_group_event_hook)(const u8 *mac_src, const u8 *mac_dst, const char *dev_name, int is_leave);

#if defined (CONFIG_MT7530_GSW) || defined (CONFIG_RALINK_MT7620)

static int
esw_mac_table_lookup(const u8 *mac, u32 cvid, int static_only)
{
	int i, j;
	u32 value[3], mac_esw[2], mac_src[2], atc_val, port_mask;

	memcpy(&mac_src[0], mac+0, 4);
	memcpy(&mac_src[1], mac+4, 2);
	mac_src[1] &= 0xffff;

	atc_val = 0x8604; // all static

	if (!static_only) {
		atc_val = 0x8a04; // filter MAC by CVID
		esw_reg_set(REG_ESW_WT_MAC_ATA2, cvid & 0xfff);
	}

	esw_reg_set(REG_ESW_WT_MAC_ATC, atc_val);

	for (i = 0; i < 0x800; i++) {
		for (j = 0; j < 100; j++) {
			usleep_range(300, 500);
			value[0] = esw_reg_get(REG_ESW_WT_MAC_ATC);
			if (value[0] & BIT(15))
				continue;
			if (value[0] & BIT(13)) {
				value[1] = esw_reg_get(REG_ESW_TABLE_ATRD);
				/* skip invalid entry */
				if ((value[1] >> 24) == 0)
					goto next_entry;
				port_mask = (value[1] >> 4) & 0x7f;
				if (!static_only) {
					/* skip P6 portmask (CPU port) */
					if (port_mask & 0x40)
						goto next_entry;
#if defined (CONFIG_MT7530_GSW)
					/* skip P5 portmask (CPU port) */
					if (port_mask & 0x20)
						goto next_entry;
#endif
				}
				value[2] = esw_reg_get(REG_ESW_TABLE_TSRA2);
				/* skip other VID */
				if (cvid != (value[2] & 0xfff))
					goto next_entry;
				mac_esw[0] = __be32_to_cpu(esw_reg_get(REG_ESW_TABLE_TSRA1));
				mac_esw[1] = __be16_to_cpu(value[2] >> 16);
				if (mac_src[0] != mac_esw[0] || mac_src[1] != mac_esw[1])
					goto next_entry;
				
				return port_mask;
			}
			else if (value[0] & BIT(14)) {
				/* end of table */
				if (((value[0] >> 16) & 0xfff) != 0x7ff)
					return -EIO;
				
				return -1;
			}
		}
next_entry:
		esw_reg_set(REG_ESW_WT_MAC_ATC, atc_val | 0x1);
	}

	return -1;
}

static int
esw_update_mcast_mask(const u8 *mcast_mac, u32 cvid, u32 port_id, int is_leave)
{
	int portmask_old, i;
	u32 portmask_new;
	u32 port_dst_msk = (1u << port_id);
	u32 uports_static = (MASK_LAN_PORT_CPU | g_igmp_sn_static_ports);
	u32 value, mac_esw[2];

	memcpy(&mac_esw[0], mcast_mac+0, 4);
	memcpy(&mac_esw[1], mcast_mac+4, 2);
	mac_esw[1] &= 0xffff;

	mac_esw[1] = __cpu_to_be16(mac_esw[1]);
	mac_esw[1] <<= 16;
	mac_esw[1] |= BIT(15);
	mac_esw[1] |= (cvid & 0xfff);

	for (i = 0; i < MAC_LOOKUP_RETRY_MAX; i++) {
		portmask_old = esw_mac_table_lookup(mcast_mac, cvid, 1);
		if (portmask_old != -EIO)
			break;
	}
	if (portmask_old < 0)
		portmask_old = 0; // entry not exist, reset mask to 0

	portmask_new = portmask_old | uports_static;

	value = 0;
	value |= (0xff << 24);
	value |= (0x03 << 2);	// static

	if (!is_leave) {
		portmask_new |= port_dst_msk;
	} else {
		portmask_new &= ~(port_dst_msk);
		/* try to remove static entry */
		if (!(portmask_new & ~uports_static) && is_leave > 1) {
			value = 0;	// invalid
			portmask_new = 0;
		}
	}

	if (portmask_new == (u32)portmask_old)
		return 0;

	value |= ((u32)portmask_new << 4);

	esw_reg_set(REG_ESW_WT_MAC_ATA1, __cpu_to_be32(mac_esw[0]));
	esw_reg_set(REG_ESW_WT_MAC_ATA2, mac_esw[1]);
	esw_reg_set(REG_ESW_WT_MAC_ATWD, value);
	esw_reg_set(REG_ESW_WT_MAC_ATC, 0x8001);

#if defined (CONFIG_MT7530_GSW)
	return mt7530_gsw_wait_wt_mac();
#else
	return mt7620_esw_wait_wt_mac();
#endif
}

static void
esw_mac_table_clear_static(void)
{
#if defined (CONFIG_MT7530_GSW)
	mt7530_gsw_mac_table_clear(1);
#else
	mt7620_esw_mac_table_clear(1);
#endif
}

#else

static int
esw_mac_table_lookup(const u8 *mac, u32 vidx, int static_only)
{
	int i, j;
	u32 value, mac_esw[2], mac_src[2], port_mask;

	memcpy(&mac_src[0], mac+0, 4);
	memcpy(&mac_src[1], mac+4, 2);
	mac_src[1] &= 0xffff;

	esw_reg_set(REG_ESW_TABLE_SEARCH, 0x1);

	for (i = 0; i < 0x3fe; i++) {
		for (j = 0; j < 100; j++) {
			usleep_range(500, 800);
			value = esw_reg_get(REG_ESW_TABLE_STATUS0);
			if (value & 0x1) {
				/* skip invalid entry */
				if ((value & 0x70) == 0)
					goto next_entry;
				/* skip other VID */
				if (vidx != ((value >> 7) & 0xf))
					goto next_entry;
				port_mask = (value >> 12) & 0x7f;
				if (!static_only) {
					/* skip P6 portmask (CPU port) */
					if (port_mask & 0x40)
						goto next_entry;
				}
				mac_esw[0] = __be32_to_cpu(esw_reg_get(REG_ESW_TABLE_STATUS2));
				mac_esw[1] = __be16_to_cpu(esw_reg_get(REG_ESW_TABLE_STATUS1) & 0xffff);
				
				if (mac_src[0] != mac_esw[0] || mac_src[1] != mac_esw[1])
					goto next_entry;
				
				return port_mask;
			}
			else if (value & 0x2) {
				/* end of table */
				return -1;
			}
		}
next_entry:
		esw_reg_set(REG_ESW_TABLE_SEARCH, 0x2);
	}

	return -1;
}

static int
esw_update_mcast_mask(const u8 *mcast_mac, u32 vidx, u32 port_id, int is_leave)
{
	int portmask_old;
	u32 portmask_new;
	u32 port_dst_msk = (1u << port_id);
	u32 uports_static = (MASK_ESW_PORT_CPU | g_igmp_sn_static_ports);
	u32 value, mac_esw[2];

	memcpy(&mac_esw[0], mcast_mac+0, 4);
	memcpy(&mac_esw[1], mcast_mac+4, 2);
	mac_esw[1] &= 0xffff;

	portmask_old = esw_mac_table_lookup(mcast_mac, vidx, 1);
	if (portmask_old < 0)
		portmask_old = 0; // entry not exist, reset mask to 0

	portmask_new = portmask_old | uports_static;

	value = 1;
	value |= (vidx << 7);
	value |= (7 << 4);	// static

	if (!is_leave) {
		portmask_new |= port_dst_msk;
	} else {
		portmask_new &= ~(port_dst_msk);
		/* try to remove static entry */
		if (!(portmask_new & ~uports_static) && is_leave > 1) {
			value &= ~(7 << 4);	// invalid
			portmask_new = 0;
		}
	}

	if (portmask_new == (u32)portmask_old)
		return 0;

	value |= ((u32)portmask_new << 12);

	esw_reg_set(REG_ESW_WT_MAC_AD2, __cpu_to_be32(mac_esw[0]));
	esw_reg_set(REG_ESW_WT_MAC_AD1, __cpu_to_be16(mac_esw[1]));
	esw_reg_set(REG_ESW_WT_MAC_AD0, value);

	return rt305x_esw_wait_wt_mac();
}

static void
esw_mac_table_clear_static(void)
{
	rt305x_esw_mac_table_clear(1);
}
#endif

static int
esw_find_port_with_ucast_mac(const u8 *ucast_mac, u16 port_cvid)
{
	int i, port_map;

	for (i = 0; i < MAC_LOOKUP_RETRY_MAX; i++) {
		port_map = esw_mac_table_lookup(ucast_mac, port_cvid, 0);
		if (port_map != -EIO)
			break;
	}

	if (port_map < 1)
		return -1;

	switch(port_map)
	{
	case BIT(0):
		return 0;
	case BIT(1):
		return 1;
	case BIT(2):
		return 2;
	case BIT(3):
		return 3;
	case BIT(4):
		return 4;
#if !defined (CONFIG_MT7530_GSW)
	case BIT(5):
		return 5;
#endif
	}

	return -1;
}

static struct mcast_group_entry*
get_empty_group_entry_from_pool(void)
{
	int i;
	struct mcast_group_entry* mge;

	for (i = 0; i < MCAST_TABLE_MAX_SIZE; i++) {
		mge = &g_mtb.pool[i];
		if (!mge->valid) {
			memset(mge, 0, sizeof(struct mcast_group_entry));
			return mge;
		}
	}

	return NULL;
}

static struct mcast_member_entry*
lookup_mcast_member_entry(struct mcast_group_entry* mge, const u8 *haddr, u32 port_id, int create_if_none)
{
	struct mcast_member_entry *mme, **prev;

	for (prev = &mge->members[port_id], mme = *prev; mme; prev = &mme->next, mme = *prev) {
		if (ether_addr_equal(mme->haddr, haddr))
			return mme;
	}

	if (!create_if_none)
		return NULL;

	/* create new member entry */
	mme = kzalloc(sizeof(struct mcast_member_entry), GFP_KERNEL);
	if (mme) {
		memcpy(mme->haddr, haddr, ETH_ALEN);
		
		/* add entry to list */
		*prev = mme;
	} else {
		if (net_ratelimit())
			printk("%s - unable to allocate member entry for IGMP/MLD group table!\n", MTK_ESW_DEVNAME);
	}

	return mme;
}

static struct mcast_group_entry*
lookup_mcast_group_entry(const u8 *maddr, u16 port_cvid, int create_if_none)
{
	u32 hash_idx;
	struct mcast_group_entry *mge, **prev;

	hash_idx = MCAST_ADDR_HASH_INDEX(maddr);

	for (prev = &g_mtb.hash[hash_idx], mge = *prev; mge; prev = &mge->next, mge = *prev) {
		if (mge->cvid == port_cvid && ether_addr_equal(mge->maddr, maddr))
			return mge;
	}

	if (!create_if_none)
		return NULL;

	/* create new group entry */
	mge = get_empty_group_entry_from_pool();
	if (mge) {
		mge->valid = 1;
		mge->cvid = port_cvid;
		memcpy(mge->maddr, maddr, ETH_ALEN);
		
		/* add entry to list */
		*prev = mge;
	} else {
		if (net_ratelimit())
			printk("%s - IGMP/MLD group table is full (max: %d entries)\n", MTK_ESW_DEVNAME, MCAST_TABLE_MAX_SIZE);
	}

	return mge;
}

static inline int
mcast_group_has_members_for_port(struct mcast_group_entry *mge, u32 port_id)
{
	struct mcast_member_entry *mme;

	/* check group members exist for this port */
	for (mme = mge->members[port_id]; mme; mme = mme->next) {
		if (mme->entered)
			return 1;
	}

	return 0;
}

static int
mcast_group_has_members(struct mcast_group_entry *mge)
{
	u32 port_id;

	/* check group members exist for each port */
	for (port_id = 0; port_id <= ESW_EPHY_ID_MAX; port_id++) {
		if (mcast_group_has_members_for_port(mge, port_id))
			return 1;
	}

	return 0;
}

static void
mcast_group_members_clear(struct mcast_group_entry *mge, int update_esw_port)
{
	u32 port_id;
	int port_has_members;
	struct mcast_member_entry *mme, *next;

	for (port_id = 0; port_id <= ESW_EPHY_ID_MAX; port_id++) {
		port_has_members = 0;
		for (mme = mge->members[port_id]; mme; mme = next) {
			next = mme->next;
			if (mme->entered)
				port_has_members = 1;
			kfree(mme);
		}
		mge->members[port_id] = NULL;
		
		if (update_esw_port && port_has_members) {
			mutex_lock(&g_lut_lock);
			esw_update_mcast_mask(mge->maddr, mge->cvid, port_id, 1);
			mutex_unlock(&g_lut_lock);
		}
	}
}

static void
mcast_table_clear(void)
{
	int i;

	for (i = 0; i < MCAST_TABLE_MAX_SIZE; i++) {
		if (g_mtb.pool[i].valid)
			mcast_group_members_clear(&g_mtb.pool[i], 0);
	}

	memset(&g_mtb, 0, sizeof(struct mcast_table));
}

static void
on_mcast_group_event(const u8 *maddr, const u8 *haddr, u16 port_cvid, u32 port_id, int is_leave)
{
	int retVal, port_members_exist = 0;
	struct mcast_group_entry *mge;
	struct mcast_member_entry *mme;

	mutex_lock(&g_mtb_lock);

	/* find group for this [addr+efid+fid] or create new if not found (only for enter) */
	mge = lookup_mcast_group_entry(maddr, port_cvid, !is_leave);
	if (!mge)
		goto table_unlock;

	if (!is_leave) {
		/* update group enter time */
		mge->last_time = jiffies;
		
		/* fing at least one group member this port before enter */
		port_members_exist = mcast_group_has_members_for_port(mge, port_id);
	}

	/* find group member for this port or create new if not found (only for enter) */
	mme = lookup_mcast_member_entry(mge, haddr, port_id, !is_leave);
	if (mme) {
		/* update group member entered state */
		mme->entered = (is_leave) ? 0 : 1;
	}

	/* fing at least one group member this port after leaved */
	if (is_leave)
		port_members_exist = mcast_group_has_members_for_port(mge, port_id);

table_unlock:

	mutex_unlock(&g_mtb_lock);

	/*
	    for enter: check what this is fist member for this port
	    for leave: check what all members for this port is leaved from group
	*/
	if (port_members_exist)
		return;

	/* update hardware multicast MAC port mask in L2 lookup table */
	mutex_lock(&g_lut_lock);
	retVal = esw_update_mcast_mask(maddr, port_cvid, port_id, is_leave);
	mutex_unlock(&g_lut_lock);

	if (retVal != 0) {
		if (net_ratelimit())
			printk("%s - unable to update LUT table (cvid: %d) for mcast group %02X-%02X-%02X-%02X-%02X-%02X!\n",
				MTK_ESW_DEVNAME, port_cvid,
				maddr[0], maddr[1], maddr[2],
				maddr[3], maddr[4], maddr[5]);
	}
}

static void
on_membership_work(struct work_struct *work)
{
	u32 hash_idx;
	int group_cleared;
	unsigned long now;
	struct mcast_group_entry *mge, **prev;

	if (!g_igmp_sn_enabled)
		return;

	mutex_lock(&g_mtb_lock);

	now = jiffies;

	for (hash_idx = 0; hash_idx < MCAST_TABLE_HASH_SIZE; hash_idx++) {
		for (prev = &g_mtb.hash[hash_idx], mge = *prev; mge; mge = *prev) {
			group_cleared = 0;
			if (!mcast_group_has_members(mge)) {
				/* no members for this group */
				mcast_group_members_clear(mge, 0);
				group_cleared = 1;
#ifdef IGMP_SN_DBG
				printk("%s - mcast group has no members, clean garbage. cvid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					MTK_ESW_DEVNAME, mge->cvid,
					mge->maddr[0], mge->maddr[1], mge->maddr[2],
					mge->maddr[3], mge->maddr[4], mge->maddr[5]);
#endif
			} else if (time_after(now, mge->last_time + MCAST_GROUP_MEMBERSHIP_EXPIRED)) {
				/* time expired for this group */
				mcast_group_members_clear(mge, 1);
				group_cleared = 1;
#ifdef IGMP_SN_DBG
				printk("%s - mcast group expired! (time after: %ld), cvid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					MTK_ESW_DEVNAME, (now - mge->last_time), mge->cvid,
					mge->maddr[0], mge->maddr[1], mge->maddr[2],
					mge->maddr[3], mge->maddr[4], mge->maddr[5]);
#endif
			}
			
			if (group_cleared) {
				/* remove empty group from hash table */
				*prev = mge->next;
				mge->valid = 0;
				mge->next = NULL;
			} else {
				prev = &mge->next;
			}
		}
	}

	if (g_igmp_sn_enabled)
		mod_timer(&g_membership_expired_timer, now + MCAST_GROUP_CHECK_INERVAL);

	mutex_unlock(&g_mtb_lock);
}

static DECLARE_WORK(g_membership_expired_work, on_membership_work);

static void
on_membership_timer(unsigned long data)
{
	if (!g_igmp_sn_enabled)
		return;

	schedule_work(&g_membership_expired_work);
}

static void
mcast_group_event_wq(struct work_struct *work)
{
	int port_id;
	u32 lan_grp_mask;
	struct mcast_work *mw = container_of(work, struct mcast_work, ws);

#ifdef IGMP_SN_DBG
	const char *s_event = "enter";

	if (mw->is_leave == 2)
		s_event = "leave_last";
	else if (mw->is_leave == 1)
		s_event = "leave";
	printk("mcast_group_%s(SRC: %02X-%02X-%02X-%02X-%02X-%02X, DST: %02X-%02X-%02X-%02X-%02X-%02X, cvid: %d)\n", 
			s_event,
			mw->mac_src[0], mw->mac_src[1], mw->mac_src[2],
			mw->mac_src[3], mw->mac_src[4], mw->mac_src[5],
			mw->mac_dst[0], mw->mac_dst[1], mw->mac_dst[2],
			mw->mac_dst[3], mw->mac_dst[4], mw->mac_dst[5],
			mw->port_cvid
		);
#endif

	/* get port_id by unicast source MAC in l2 lookup table */
	mutex_lock(&g_lut_lock);
	port_id = esw_find_port_with_ucast_mac(mw->mac_src, mw->port_cvid);
	mutex_unlock(&g_lut_lock);

	/* check valid port_id */
	if (port_id >= 0 && port_id <= ESW_EPHY_ID_MAX) {
#if defined (CONFIG_MT7530_GSW) || defined (CONFIG_RALINK_MT7620)
		lan_grp_mask = get_ports_mask_lan(0, 0);
#else
		lan_grp_mask = get_ports_mask_lan(0);
#endif
		/* check this port_id in LAN group */
		if ((1u << port_id) & lan_grp_mask)
			on_mcast_group_event(mw->mac_dst, mw->mac_src, mw->port_cvid, (u32)port_id, (int)mw->is_leave);
	}

	kfree(mw);
}

static void
esw_mcast_group_event(const u8 *mac_src, const u8 *mac_dst, const char *dev_name, int is_leave)
{
	struct mcast_work *mw;
	u16 port_cvid;

	if (!g_igmp_sn_enabled)
		return;

	/* check source MAC is unicast */
	if (mac_src[0] & 0x01)
		return;

	/* check destination MAC is mapped as some reserved IPv4 group (01:00:5E:00:00:XX) */
	if (mac_dst[0] == 0x01 &&
	    mac_dst[3] == 0x00 &&
	    mac_dst[4] == 0x00 &&
	   (mac_dst[5] <= 0x02 || mac_dst[5] == 0x16 || mac_dst[5] >= 0xFB))
		return;

	/* check destination MAC is mapped as some reserved IPv6 group (33:33:00:00:00:XX) */
	if (mac_dst[0] == 0x33 &&
	    mac_dst[2] == 0x00 &&
	    mac_dst[3] == 0x00 &&
	    mac_dst[4] == 0x00 &&
	   (mac_dst[5] <= 0x16 || mac_dst[5] == 0xFB))
		return;

	/* listen eth2.x events only */
	if (strncmp(dev_name, "eth2", 4) != 0)
		return;

#if defined (CONFIG_MT7530_GSW) || defined (CONFIG_RALINK_MT7620)
	port_cvid = 1;	// always use cvid = 1 for LAN group
#else
	port_cvid = 0;	// always use vidx = 0 for LAN group (VID=1)
#endif

	/* create work (do not touch MDIO registers in softirq context) */
	mw = (struct mcast_work *)kmalloc(sizeof(struct mcast_work), GFP_ATOMIC);
	if (mw) {
		INIT_WORK(&mw->ws, mcast_group_event_wq);
		memcpy(mw->mac_src, mac_src, ETH_ALEN);
		memcpy(mw->mac_dst, mac_dst, ETH_ALEN);
		mw->is_leave = (u16)is_leave;
		mw->port_cvid = port_cvid;
		if (!schedule_work(&mw->ws))
			kfree(mw);
	}
}

static void
mcast_reset_table_and_cancel(void)
{
	del_timer_sync(&g_membership_expired_timer);
	cancel_work_sync(&g_membership_expired_work);

	mutex_lock(&g_lut_lock);
	esw_mac_table_clear_static();
	mutex_unlock(&g_lut_lock);

	mutex_lock(&g_mtb_lock);
	mcast_table_clear();
	mutex_unlock(&g_mtb_lock);
}

void
igmp_sn_set_enable(u32 igmp_sn_enabled)
{
	if (igmp_sn_enabled)
		igmp_sn_enabled = 1;

	if (g_igmp_sn_enabled != igmp_sn_enabled) {
		g_igmp_sn_enabled = igmp_sn_enabled;
		if (!igmp_sn_enabled) {
			mcast_reset_table_and_cancel();
			esw_igmp_flood_to_cpu(0);
		} else {
			esw_igmp_flood_to_cpu( (g_igmp_sn_static_ports) ? 0 : 1);
			
			/* set expired timer at least group_expired time */
			mod_timer(&g_membership_expired_timer, jiffies + (MCAST_GROUP_MEMBERSHIP_EXPIRED + 10 * HZ));
		}
		
		printk("%s - software IGMP/MLD snooping: %d\n", MTK_ESW_DEVNAME, igmp_sn_enabled);
	}
}

void
igmp_sn_set_static_ports(u32 ports_mask)
{
	u32 lan_grp_mask;
	int flood_to_cpu = 0;

#if defined (CONFIG_MT7530_GSW) || defined (CONFIG_RALINK_MT7620)
	lan_grp_mask = get_ports_mask_lan(0, 0);
#else
	lan_grp_mask = get_ports_mask_lan(0);
#endif
	g_igmp_sn_static_ports = (ports_mask & lan_grp_mask);

	if (g_igmp_sn_enabled && !g_igmp_sn_static_ports)
		flood_to_cpu = 1;
	esw_igmp_flood_to_cpu(flood_to_cpu);
}

void
igmp_sn_init(void)
{
	memset(&g_mtb, 0, sizeof(struct mcast_table));
	setup_timer(&g_membership_expired_timer, on_membership_timer, 0);

	br_mcast_group_event_hook = esw_mcast_group_event;
}

void
igmp_sn_uninit(void)
{
	br_mcast_group_event_hook = NULL;

	g_igmp_sn_enabled = 0;

	mcast_reset_table_and_cancel();

	flush_scheduled_work();
}

