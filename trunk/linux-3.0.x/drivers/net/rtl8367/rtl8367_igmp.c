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

#define MCAST_TABLE_MAX_SIZE		256
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
	u16				cvid_fid:12;
	u8				maddr[ETHER_ADDR_LEN];
};

struct mcast_table
{
	struct mcast_group_entry*	hash[MCAST_TABLE_HASH_SIZE];
	struct mcast_group_entry	pool[MCAST_TABLE_MAX_SIZE];
};

struct mcast_work
{
	struct work_struct ws;
	u8 mac_src[ETHER_ADDR_LEN];
	u8 mac_dst[ETHER_ADDR_LEN];
	u16 is_leave:2;
	u16 port_efid:2;	// always 0 for LAN group
	u16 port_cvid_fid:12;
};

static DEFINE_MUTEX(g_lut_lock);
static DEFINE_MUTEX(g_mtb_lock);
static struct mcast_table g_mtb;
static struct timer_list g_membership_expired_timer;
static u32 g_igmp_snooping_enabled = 0;
static u32 g_igmp_static_ports = 0;
#if defined(CONFIG_RTL8367_API_8370)
static DECLARE_BITMAP(g_l2t_cache, RTK_MAX_NUM_OF_LEARN_LIMIT);
#endif

extern void (*br_mcast_group_event_hook)(const u8 *mac_src, const u8 *mac_dst, const char *dev_name, int is_leave);

static int
asic_find_port_with_ucast_mac(const u8 *ucast_mac, u16 port_efid, u16 port_cvid_fid)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_cvid_fid;	// svl, mean fid
	if (rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.ivl_svl = 1;
	l2t.cvid_fid = port_cvid_fid;	// ivl, mean cvid
	if (rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#endif
	return (int)l2t.spa;
}

static rtk_api_ret_t
asic_update_mcast_mask(const u8 *mcast_mac, u16 port_efid, u16 port_cvid_fid, u32 port_id, int is_leave)
{
	rtk_api_ret_t retVal;
	u16 portmask_old;
	u16 port_dst_msk = (1u << port_id);
	u16 uports_static = MASK_LAN_PORT_CPU | (u16)g_igmp_static_ports;
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.fid = port_cvid_fid;	// svl, mean fid
	retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal == RT_ERR_OK) {
		if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT) {
			if (l2t.portmask)
				set_bit(l2t.address, g_l2t_cache);
			else
				clear_bit(l2t.address, g_l2t_cache);
		}
	} else
		l2t.portmask = 0;	// entry not exist, reset mask to 0

	portmask_old = l2t.portmask;

	l2t.ipmul = 0;
	l2t.static_bit = 1;
	l2t.portmask |= uports_static;

	if (!is_leave) {
		l2t.portmask |= port_dst_msk;
	} else {
		l2t.portmask &= ~port_dst_msk;
		/* try to remove static entry */
		if (!(l2t.portmask & ~uports_static) && is_leave > 1) {
			l2t.static_bit = 0;
			l2t.portmask = 0;
		}
	}

	if (l2t.portmask == portmask_old)
		return RT_ERR_OK;

	retVal = rtl8370_setAsicL2LookupTb(&l2t);
	if (retVal == RT_ERR_OK) {
		if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT) {
			if (l2t.portmask)
				set_bit(l2t.address, g_l2t_cache);
			else
				clear_bit(l2t.address, g_l2t_cache);
		}
	}

	return retVal;
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.efid = port_efid;
	l2t.ivl_svl = 1;
	l2t.cvid_fid = port_cvid_fid;	// ivl, mean cvid
	retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal != RT_ERR_OK)
		l2t.mbr = 0; // entry not exist, reset mask to 0

	portmask_old = l2t.mbr;

	l2t.l3lookup = 0;
	l2t.nosalearn = 1;
	l2t.mbr |= uports_static;

	if (!is_leave) {
		l2t.mbr |= port_dst_msk;
	} else {
		l2t.mbr &= ~port_dst_msk;
		/* try to remove static entry */
		if (!(l2t.mbr & ~uports_static) && is_leave > 1) {
			l2t.nosalearn = 0;
			l2t.mbr = 0;
		}
	}

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
		if (ether_addr_equal(mme->haddr, haddr))
			return mme;
	}

	if (!create_if_none)
		return NULL;

	/* create new member entry */
	mme = kzalloc(sizeof(struct mcast_member_entry), GFP_KERNEL);
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
lookup_mcast_group_entry(const u8 *maddr, u16 port_efid, u16 port_cvid_fid, int create_if_none)
{
	u32 hash_idx;
	struct mcast_group_entry *mge, **prev;

	hash_idx = MCAST_ADDR_HASH_INDEX(maddr);

	for (prev = &g_mtb.hash[hash_idx], mge = *prev; mge; prev = &mge->next, mge = *prev) {
		if (mge->efid == port_efid && mge->cvid_fid == port_cvid_fid && ether_addr_equal(mge->maddr, maddr))
			return mge;
	}

	if (!create_if_none)
		return NULL;

	/* create new group entry */
	mge = get_empty_group_entry_from_pool();
	if (mge) {
		mge->valid = 1;
		mge->efid = port_efid;
		mge->cvid_fid = port_cvid_fid;
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
			mutex_lock(&g_lut_lock);
			asic_update_mcast_mask(mge->maddr, mge->efid, mge->cvid_fid, port_id, 1);
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
on_mcast_group_event(const u8 *maddr, const u8 *haddr, u16 port_efid, u16 port_cvid_fid, u32 port_id, int is_leave)
{
	int port_members_exist = 0;
	rtk_api_ret_t retVal;
	struct mcast_group_entry *mge;
	struct mcast_member_entry *mme;

	mutex_lock(&g_mtb_lock);

	/* find group for this [addr+efid+fid] or create new if not found (only for enter) */
	mge = lookup_mcast_group_entry(maddr, port_efid, port_cvid_fid, !is_leave);
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
	retVal = asic_update_mcast_mask(maddr, port_efid, port_cvid_fid, port_id, is_leave);
	mutex_unlock(&g_lut_lock);

	if (retVal != RT_ERR_OK) {
		if (net_ratelimit())
			printk("%s - unable to update LUT table (efid: %d, cvid_fid: %d) for mcast group %02X-%02X-%02X-%02X-%02X-%02X!\n",
				RTL8367_DEVNAME, port_efid, port_cvid_fid,
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

	if (!g_igmp_snooping_enabled)
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
#ifdef RTL8367_DBG
				printk("%s - mcast group has no members, clean garbage. efid: %d, cvid_fid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					RTL8367_DEVNAME, mge->efid, mge->cvid_fid,
					mge->maddr[0], mge->maddr[1], mge->maddr[2],
					mge->maddr[3], mge->maddr[4], mge->maddr[5]);
#endif
			} else if (time_after(now, mge->last_time + MCAST_GROUP_MEMBERSHIP_EXPIRED)) {
				/* time expired for this group */
				mcast_group_members_clear(mge, 1);
				group_cleared = 1;
#ifdef RTL8367_DBG
				printk("%s - mcast group expired! (time after: %ld), efid: %d, cvid_fid: %d, group: %02X-%02X-%02X-%02X-%02X-%02X\n", 
					RTL8367_DEVNAME, (now - mge->last_time), mge->efid, mge->cvid_fid,
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

	mutex_unlock(&g_mtb_lock);
}

static DECLARE_WORK(g_membership_expired_work, on_membership_work);

static void
on_membership_timer(unsigned long data)
{
	if (!g_igmp_snooping_enabled)
		return;

	schedule_work(&g_membership_expired_work);
}

static void
asic_enum_mcast_table(int clear_all)
{
	rtk_api_ret_t retVal;
	int index = 0;
	u32 addr_id = 0;
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	for (addr_id = 0; addr_id < RTK_MAX_NUM_OF_LEARN_LIMIT; addr_id++) {
		if (!test_bit(addr_id, g_l2t_cache))
			continue;
		
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_ADDRESS, &l2t);
		if (retVal != RT_ERR_OK)
			continue;
		
		if (!(l2t.mac.octet[0] & 0x01))
			continue;
		
		if (clear_all) {
			l2t.ipmul = 0;
			l2t.block = 0;
			l2t.static_bit = 0;
			l2t.portmask = 0;
			rtl8370_setAsicL2LookupTb(&l2t);
		} else {
			index++;
			printk("%4d. %02X-%02X-%02X-%02X-%02X-%02X, portmask: 0x%04X, efid: %d, fid: %4d\n",
				index,
				l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
				l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
				l2t.portmask,
				l2t.efid,
				l2t.fid
			);
		}
	}

	if (clear_all) {
		bitmap_zero(g_l2t_cache, RTK_MAX_NUM_OF_LEARN_LIMIT);
	}
#else
	rtl8367b_luttb l2t;

	while (addr_id <= RTK_MAX_LUT_ADDR_ID) {
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L2MC, &l2t);
		if (retVal != RT_ERR_OK)
			break;
		if (l2t.address < addr_id)
			break;
		
		addr_id = l2t.address + 1;
		
		if (clear_all) {
			l2t.l3lookup = 0;
			l2t.nosalearn = 0;
			l2t.sa_block = 0;
			l2t.mbr = 0;
			rtl8367b_setAsicL2LookupTb(&l2t);
		} else {
			index++;
			printk("%4d. %02X-%02X-%02X-%02X-%02X-%02X, addr_id: %d, portmask: 0x%04X, efid: %d, cvid: %4d, ivl: %d\n",
				index,
				l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
				l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
				l2t.address,
				l2t.mbr,
				l2t.efid,
				l2t.cvid_fid,
				l2t.ivl_svl
			);
		}
	}
#endif
}

static void
mcast_group_event_wq(struct work_struct *work)
{
	int port_id;
	u32 lan_grp_mask;
	struct mcast_work *mw = container_of(work, struct mcast_work, ws);

#ifdef RTL8367_DBG
	const char *s_event = "enter";

	if (mw->is_leave == 2)
		s_event = "leave_last";
	else if (mw->is_leave == 1)
		s_event = "leave";

	printk("rtl8367_mcast_group_%s(SRC: %02X-%02X-%02X-%02X-%02X-%02X, DST: %02X-%02X-%02X-%02X-%02X-%02X, cvid_fid: %d)\n", 
			s_event,
			mw->mac_src[0], mw->mac_src[1], mw->mac_src[2],
			mw->mac_src[3], mw->mac_src[4], mw->mac_src[5],
			mw->mac_dst[0], mw->mac_dst[1], mw->mac_dst[2],
			mw->mac_dst[3], mw->mac_dst[4], mw->mac_dst[5],
			mw->port_cvid_fid
		);
#endif

	/* get port_id by unicast source MAC in l2 lookup table */
	mutex_lock(&g_lut_lock);
	port_id = asic_find_port_with_ucast_mac(mw->mac_src, mw->port_efid, mw->port_cvid_fid);
	mutex_unlock(&g_lut_lock);

	/* check valid port_id */
	if (port_id >= 0 && port_id < RTK_MAX_NUM_OF_PORT) {
		lan_grp_mask = get_phy_ports_mask_lan(0);
#if defined(EXT_PORT_INIC)
		lan_grp_mask |= (1u << EXT_PORT_INIC);
#endif
		/* check this port_id in LAN group */
		if ((1u << port_id) & lan_grp_mask)
			on_mcast_group_event(mw->mac_dst, mw->mac_src, mw->port_efid, mw->port_cvid_fid, (u32)port_id, (int)mw->is_leave);
	}

	kfree(mw);
}

static void
rtl8367_mcast_group_event(const u8 *mac_src, const u8 *mac_dst, const char *dev_name, int is_leave)
{
	struct mcast_work *mw;
	u16 port_efid, port_cvid_fid;

	if (!g_igmp_snooping_enabled)
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

	/* listen eth2/eth2.1/eth2.3 events only */
	if (strncmp(dev_name, "eth2", 4) != 0)
		return;

	port_efid = 0;		// always use efid = 0 for LAN group
	port_cvid_fid = 1;	// always use fid = 1 (cvid = 1) for LAN group
#if defined(EXT_PORT_INIC)
	if (strcmp(dev_name, "eth2.3") == 0) {
#if defined(CONFIG_RTL8367_API_8370)
		port_cvid_fid = INIC_GUEST_VLAN_FID;	// use for SVL
#else
		port_cvid_fid = INIC_GUEST_VLAN_VID;	// use for IVL
#endif
	}
#endif

	/* create work (do not touch MDIO registers in softirq context) */
	mw = (struct mcast_work *)kmalloc(sizeof(struct mcast_work), GFP_ATOMIC);
	if (mw) {
		INIT_WORK(&mw->ws, mcast_group_event_wq);
		memcpy(mw->mac_src, mac_src, ETHER_ADDR_LEN);
		memcpy(mw->mac_dst, mac_dst, ETHER_ADDR_LEN);
		mw->is_leave = (u16)is_leave;
		mw->port_efid = port_efid;
		mw->port_cvid_fid = port_cvid_fid;
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
	asic_enum_mcast_table(1);
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

	if (g_igmp_snooping_enabled != igmp_sn_enabled) {
		g_igmp_snooping_enabled = igmp_sn_enabled;
		if (!igmp_sn_enabled) {
			mcast_reset_table_and_cancel();
		} else {
			/* set expired timer at least group_expired time */
			mod_timer(&g_membership_expired_timer, jiffies + (MCAST_GROUP_MEMBERSHIP_EXPIRED + 10 * HZ));
		}
		printk("%s - IGMP/MLD snooping: %d\n", RTL8367_DEVNAME, igmp_sn_enabled);
	}
}

void
igmp_sn_set_static_ports(u32 ports_mask)
{
	u32 lan_grp_mask = get_phy_ports_mask_lan(0);

	g_igmp_static_ports = get_ports_mask_from_uapi(ports_mask) & lan_grp_mask;
}

void
igmp_sn_dump_mcast_table(void)
{
	printk("%s - dump multicast LUT table:\n", RTL8367_DEVNAME);

	mutex_lock(&g_lut_lock);
	asic_enum_mcast_table(0);
	mutex_unlock(&g_lut_lock);
}

void
igmp_sn_init(void)
{
	memset(&g_mtb, 0, sizeof(struct mcast_table));
#if defined(CONFIG_RTL8367_API_8370)
	bitmap_zero(g_l2t_cache, RTK_MAX_NUM_OF_LEARN_LIMIT);
#endif
	setup_timer(&g_membership_expired_timer, on_membership_timer, 0);

	br_mcast_group_event_hook = rtl8367_mcast_group_event;
}

void
igmp_sn_uninit(void)
{
	br_mcast_group_event_hook = NULL;
	g_igmp_snooping_enabled = 0;

	mcast_reset_table_and_cancel();

	flush_scheduled_work();
}

