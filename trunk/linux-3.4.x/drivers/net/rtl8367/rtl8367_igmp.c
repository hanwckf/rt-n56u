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

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_types.h"
#include "api_8370/rtk_error.h"
#include "api_8370/rtk_api_ext.h"
#include "api_8370/rtl8370_asicdrv_lut.h"
#else
#include "api_8367b/rtk_types.h"
#include "api_8367b/rtk_error.h"
#include "api_8367b/rtk_api_ext.h"
#include "api_8367b/rtl8367b_asicdrv_lut.h"
#endif

#include "rtl8367_def.h"
#include "rtl8367_ioctl.h"

#define MCAST_TABLE_MAX_SIZE		512
#define MCAST_TABLE_HASH_SIZE		128 // must be power of 2 and <= 256
#define MCAST_GROUP_CHECK_INERVAL	 (33 * HZ)
#define MCAST_GROUP_MEMBERSHIP_EXPIRED	(260 * HZ) // (125 * 2) + 10

#define MCAST_ADDR_HASH(addr)		(addr[0]^addr[1]^addr[2]^addr[3]^addr[4]^addr[5])
#define MCAST_ADDR_HASH_INDEX(addr)	(MCAST_ADDR_HASH(addr) & (MCAST_TABLE_HASH_SIZE - 1))

struct mcast_member_entry
{
	struct mcast_member_entry*	next;
	u16				entered;
	u8				haddr[ETHER_ADDR_LEN];
};

struct mcast_group_entry
{
	struct mcast_group_entry*	next;
	struct mcast_member_entry*	members[RTK_MAX_NUM_OF_PORT];
	unsigned long			last_time;
	u16				valid:1;
	u16				efid:3;
	u16				fid:12;
	u8				maddr[ETHER_ADDR_LEN];
};

struct mcast_table
{
	struct mcast_group_entry*	hash[MCAST_TABLE_HASH_SIZE];
	struct mcast_group_entry	pool[MCAST_TABLE_MAX_SIZE];
};

static spinlock_t g_lut_lock;
static spinlock_t g_mtb_lock;
static struct mcast_table g_mtb;
static struct timer_list g_membership_expired_timer;
static u32 g_igmp_snooping_enabled = 0;
#if defined(CONFIG_RTL8367_API_8370)
static u8 g_l2t_cache[RTK_MAX_NUM_OF_LEARN_LIMIT];
#endif

static int
asic_find_port_with_ucast_mac(const u8 *ucast_mac, u16 port_efid, u16 port_fid)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_fid;
	if (rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_fid;
	l2t.cvid_fid = port_fid; // ivl = 0, mean fid
	if (rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#endif
	return (int)l2t.spa;
}

static void
asic_delete_mcast_mask(const u8 *mcast_mac, u16 port_efid, u16 port_fid)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_fid;
	rtl8370_setAsicL2LookupTb(&l2t);
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.cvid_fid = port_fid; // ivl = 0, mean fid
	rtl8367b_setAsicL2LookupTb(&l2t);
#endif
}

static rtk_api_ret_t
asic_update_mcast_mask(const u8 *mcast_mac, u16 port_efid, u16 port_fid, u32 port_id, int is_leave)
{
	rtk_api_ret_t retVal;
	u16 portmask_old;
	u16 port_cpu_msk = (1u << LAN_PORT_CPU);
	u16 port_dst_msk = (1u << port_id);
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_fid;
	retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal == RT_ERR_OK) {
		if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT)
			g_l2t_cache[l2t.address] = 1;
	}
	else
		l2t.portmask = 0; // entry not exist, reset mask to 0

	l2t.efid = port_efid;
	l2t.fid = port_fid;
	l2t.ipmul = 0;
	l2t.static_bit = 1;

	portmask_old = l2t.portmask;

	l2t.portmask |= port_cpu_msk;
	if (!is_leave)
		l2t.portmask |= port_dst_msk;
	else
		l2t.portmask &= ~port_dst_msk;

	if (l2t.portmask == portmask_old)
		return RT_ERR_OK;

	retVal = rtl8370_setAsicL2LookupTb(&l2t);
	if (retVal == RT_ERR_OK) {
		if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT)
			g_l2t_cache[l2t.address] = 1;
	}

	return retVal;
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.cvid_fid  = port_fid; // ivl = 0, mean fid
	retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal != RT_ERR_OK)
		l2t.mbr = 0; // entry not exist, reset mask to 0

	l2t.efid = port_efid;
	l2t.ivl_svl = 0;
	l2t.cvid_fid = port_fid;
	l2t.l3lookup = 0;
	l2t.nosalearn = 1;

	portmask_old = l2t.mbr;

	l2t.mbr |= port_cpu_msk;
	if (!is_leave)
		l2t.mbr |= port_dst_msk;
	else
		l2t.mbr &= ~port_dst_msk;

	if (l2t.mbr == portmask_old)
		return RT_ERR_OK;

	return rtl8367b_setAsicL2LookupTb(&l2t);
#endif
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
		if (compare_ether_addr(mme->haddr, haddr) == 0)
			return mme;
	}

	if (!create_if_none)
		return NULL;

	/* create new member entry */
	mme = kzalloc(sizeof(struct mcast_member_entry), GFP_ATOMIC);
	if (mme) {
		memcpy(mme->haddr, haddr, ETHER_ADDR_LEN);
		
		/* add entry to list */
		*prev = mme;
	} else {
		if (net_ratelimit())
			printk("%s - unable to allocate member entry for IGMP/MLD group table!\n", RTL8367_DEVNAME);
	}

	return mme;
}

static struct mcast_group_entry*
lookup_mcast_group_entry(const u8 *maddr, u16 port_efid, u16 port_fid, int create_if_none)
{
	u32 hash_idx;
	struct mcast_group_entry *mge, **prev;

	hash_idx = MCAST_ADDR_HASH_INDEX(maddr);

	for (prev = &g_mtb.hash[hash_idx], mge = *prev; mge; prev = &mge->next, mge = *prev) {
		if (mge->efid == port_efid && mge->fid == port_fid && compare_ether_addr(mge->maddr, maddr) == 0)
			return mge;
	}

	if (!create_if_none)
		return NULL;

	/* create new group entry */
	mge = get_empty_group_entry_from_pool();
	if (mge) {
		mge->valid = 1;
		mge->efid = port_efid;
		mge->fid = port_fid;
		memcpy(mge->maddr, maddr, ETHER_ADDR_LEN);
		
		/* add entry to list */
		*prev = mge;
	} else {
		if (net_ratelimit())
			printk("%s - IGMP/MLD group table is full (max: %d entries)\n", RTL8367_DEVNAME, MCAST_TABLE_MAX_SIZE);
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
	for (port_id = 0; port_id < RTK_MAX_NUM_OF_PORT; port_id++) {
		if (mcast_group_has_members_for_port(mge, port_id))
			return 1;
	}

	return 0;
}

static void
mcast_group_members_clear(struct mcast_group_entry *mge, int update_asic_port)
{
	u32 port_id;
	int port_has_members;
	struct mcast_member_entry *mme, *next;

	for (port_id = 0; port_id < RTK_MAX_NUM_OF_PORT; port_id++) {
		port_has_members = 0;
		for (mme = mge->members[port_id]; mme; mme = next) {
			next = mme->next;
			if (mme->entered)
				port_has_members = 1;
			kfree(mme);
		}
		mge->members[port_id] = NULL;
		
		if (update_asic_port && port_has_members) {
			spin_lock(&g_lut_lock);
			asic_update_mcast_mask(mge->maddr, mge->efid, mge->fid, port_id, 1);
			spin_unlock(&g_lut_lock);
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
on_mcast_group_event(const u8 *maddr, const u8 *haddr, u16 port_efid, u16 port_fid, u32 port_id, int is_leave)
{
	int port_members_exist = 0;
	rtk_api_ret_t retVal;
	struct mcast_group_entry *mge;
	struct mcast_member_entry *mme;

	spin_lock(&g_mtb_lock);

	/* find group for this [addr+efid+fid] or create new if not found (only for enter) */
	mge = lookup_mcast_group_entry(maddr, port_efid, port_fid, !is_leave);
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

	spin_unlock(&g_mtb_lock);

	/*
	    for enter: check what this is fist member for this port
	    for leave: check what all members for this port is leaved from group
	*/
	if (port_members_exist)
		return;

	/* update hardware multicast MAC port mask in L2 lookup table */
	spin_lock(&g_lut_lock);
	retVal = asic_update_mcast_mask(maddr, port_efid, port_fid, port_id, is_leave);
	spin_unlock(&g_lut_lock);

	if (retVal != RT_ERR_OK) {
		if (net_ratelimit())
			printk("%s - unable to update LUT table (efid: %d, fid: %d) for mcast group %02X-%02X-%02X-%02X-%02X-%02X!\n",
				RTL8367_DEVNAME, port_efid, port_fid,
				maddr[0], maddr[1], maddr[2],
				maddr[3], maddr[4], maddr[5]);
	}
}

static void
on_membership_timer(unsigned long data)
{
	u32 hash_idx;
	int group_cleared;
	unsigned long now;
	struct mcast_group_entry *mge, **prev;

	spin_lock(&g_mtb_lock);

	now = jiffies;

	for (hash_idx = 0; hash_idx < MCAST_TABLE_HASH_SIZE; hash_idx++) {
		for (prev = &g_mtb.hash[hash_idx], mge = *prev; mge; mge = *prev) {
			group_cleared = 0;
			if (!mcast_group_has_members(mge)) {
				/* no members for this group */
				mcast_group_members_clear(mge, 0);
				group_cleared = 1;
#ifdef RTL8367_DBG
				printk("%s - mcast group has no members, clean garbage. efid: %d, fid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					RTL8367_DEVNAME, mge->efid, mge->fid,
					mge->maddr[0], mge->maddr[1], mge->maddr[2],
					mge->maddr[3], mge->maddr[4], mge->maddr[5]);
#endif
			} else if (time_after(now, mge->last_time + MCAST_GROUP_MEMBERSHIP_EXPIRED)) {
				/* time expired for this group */
				mcast_group_members_clear(mge, 1);
				group_cleared = 1;
#ifdef RTL8367_DBG
				printk("%s - mcast group expired! (time after: %ld), efid: %d, fid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					RTL8367_DEVNAME, (now - mge->last_time), mge->efid, mge->fid,
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

	if (g_igmp_snooping_enabled)
		mod_timer(&g_membership_expired_timer, now + MCAST_GROUP_CHECK_INERVAL);

	spin_unlock(&g_mtb_lock);
}

static void
asic_enum_mcast_table(int clear_all)
{
	rtk_api_ret_t retVal;
	int index = 0;
	u32 addr_id = 0;
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	for (addr_id = 0; addr_id < RTK_MAX_NUM_OF_LEARN_LIMIT; addr_id++)
	{
		if (!g_l2t_cache[addr_id])
			continue;
		
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_ADDRESS, &l2t);
		if (retVal != RT_ERR_OK)
			continue;
		
		if (!(l2t.mac.octet[0] & 0x01))
			continue;
		
		if (clear_all)
		{
			asic_delete_mcast_mask(l2t.mac.octet, l2t.efid, l2t.fid);
		}
		else
		{
			index++;
			printk("  %d. %02X-%02X-%02X-%02X-%02X-%02X, portmask: 0x%04X, efid: %d, fid: %d\n", 
				index,
				l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
				l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
				l2t.portmask,
				l2t.efid,
				l2t.fid
				);
		}
	}

	if (clear_all)
		memset(g_l2t_cache, 0, sizeof(g_l2t_cache));
#else
	rtl8367b_luttb l2t;

	for (;;)
	{
		if (addr_id > RTK_MAX_LUT_ADDR_ID)
			break;
		
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L2MC, &l2t);
		if (retVal != RT_ERR_OK)
			break;
		if (l2t.address < addr_id)
			break;
		
		addr_id = l2t.address + 1;
		
		if (clear_all)
		{
			asic_delete_mcast_mask(l2t.mac.octet, l2t.efid, l2t.cvid_fid);
		}
		else
		{
			index++;
			printk("  %d. %02X-%02X-%02X-%02X-%02X-%02X, addr_id: %d, portmask: 0x%04X, efid: %d, fid: %d\n", 
				index,
				l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
				l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
				l2t.address,
				l2t.mbr,
				l2t.efid,
				l2t.cvid_fid
				);
		}
	}
#endif
}

void igmp_init(void)
{
	spin_lock_init(&g_lut_lock);
	spin_lock_init(&g_mtb_lock);
	memset(&g_mtb, 0, sizeof(struct mcast_table));
#if defined(CONFIG_RTL8367_API_8370)
	memset(g_l2t_cache, 0, sizeof(g_l2t_cache));
#endif
	setup_timer(&g_membership_expired_timer, on_membership_timer, 0);
}

void igmp_uninit(void)
{
	del_timer_sync(&g_membership_expired_timer);

	spin_lock(&g_mtb_lock);
	mcast_table_clear();
	spin_unlock(&g_mtb_lock);
}

void change_igmp_snooping_control(u32 igmp_snooping_enabled)
{
	if (igmp_snooping_enabled) igmp_snooping_enabled = 1;

	if (g_igmp_snooping_enabled != igmp_snooping_enabled)
	{
		printk("%s - IGMP/MLD snooping: %d\n", RTL8367_DEVNAME, igmp_snooping_enabled);
		
		g_igmp_snooping_enabled = igmp_snooping_enabled;
		if (!igmp_snooping_enabled) {
			del_timer_sync(&g_membership_expired_timer);
			
			spin_lock_bh(&g_lut_lock);
			asic_enum_mcast_table(1);
			spin_unlock_bh(&g_lut_lock);
			
			spin_lock_bh(&g_mtb_lock);
			mcast_table_clear();
			spin_unlock_bh(&g_mtb_lock);
		} else {
			/* set expired timer at least group_expired time */
			mod_timer(&g_membership_expired_timer, jiffies + (MCAST_GROUP_MEMBERSHIP_EXPIRED + 10 * HZ));
		}
	}
}

void reset_igmp_snooping_table(void)
{
	if (!g_igmp_snooping_enabled)
		return;

	del_timer_sync(&g_membership_expired_timer);

	spin_lock_bh(&g_lut_lock);
	asic_enum_mcast_table(1);
	spin_unlock_bh(&g_lut_lock);

	spin_lock_bh(&g_mtb_lock);
	mcast_table_clear();
	spin_unlock_bh(&g_mtb_lock);

	/* set expired timer at least group_expired time */
	mod_timer(&g_membership_expired_timer, jiffies + (MCAST_GROUP_MEMBERSHIP_EXPIRED + 10 * HZ));

	printk("%s - reset IGMP/MLD table and static LUT entries\n", RTL8367_DEVNAME);
}

void dump_mcast_table(void)
{
	printk("%s - dump multicast LUT table:\n", RTL8367_DEVNAME);

	spin_lock_bh(&g_lut_lock);
	asic_enum_mcast_table(0);
	spin_unlock_bh(&g_lut_lock);
}

void rtl8367_mcast_group_event(const u8 *mac_src, const u8 *mac_dst, const char *dev_name, int is_leave)
{
	int port_id;
	u16 port_efid, port_fid;
	u32 lan_grp_mask;

	if (!g_igmp_snooping_enabled)
		return;

#ifdef RTL8367_DBG
	printk("rtl8367_mcast_group_%s(SRC: %02X-%02X-%02X-%02X-%02X-%02X, DST: %02X-%02X-%02X-%02X-%02X-%02X, dev: %s)\n", 
			(!is_leave) ? "enter" : "leave",
			mac_src[0], mac_src[1], mac_src[2],
			mac_src[3], mac_src[4], mac_src[5],
			mac_dst[0], mac_dst[1], mac_dst[2],
			mac_dst[3], mac_dst[4], mac_dst[5],
			dev_name
		);
#endif

	/* check source MAC is unicast */
	if (mac_src[0] & 0x01)
		return;

	/* listen eth2/eth2.1/eth2.3 events only */
	if (strncmp(dev_name, "eth2", 4) != 0)
		return;

	port_efid = 0; // always use efid = 0 for LAN group
	port_fid = 1;
#if defined(EXT_PORT_INIC)
	if (strcmp(dev_name, "eth2.3") == 0)
		port_fid = INIC_GUEST_FID;
#endif

	/* get port_id by unicast source MAC in l2 lookup table */
	spin_lock(&g_lut_lock);
	port_id = asic_find_port_with_ucast_mac(mac_src, port_efid, port_fid);
	spin_unlock(&g_lut_lock);

	/* check valid port_id */
	if (port_id < 0 || port_id >= RTK_MAX_NUM_OF_PORT)
		return;

	lan_grp_mask = get_phy_ports_mask_lan(0);
#if defined(EXT_PORT_INIC)
	lan_grp_mask |= (1u << EXT_PORT_INIC);
#endif

	/* check this port_id in LAN group */
	if (!((1u << port_id) & lan_grp_mask))
		return;

	on_mcast_group_event(mac_dst, mac_src, port_efid, port_fid, (u32)port_id, is_leave);
}

EXPORT_SYMBOL(rtl8367_mcast_group_event);

