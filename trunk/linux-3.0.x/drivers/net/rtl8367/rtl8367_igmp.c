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

static spinlock_t g_lut_lock;
static u32 g_igmp_snooping_enabled = 0;
#if defined(CONFIG_RTL8367_API_8370)
static unsigned char g_l2t_cache[RTK_MAX_NUM_OF_LEARN_LIMIT];
#endif

static int
asic_find_port_with_ucast_mac(const unsigned char* ucast_mac, u32 port_fid)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.fid = (u16)port_fid;
	if (rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, ucast_mac, ETHER_ADDR_LEN);
	l2t.cvid_fid = (u16)port_fid;
	l2t.fid      = (u16)port_fid;
	if (rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t) != RT_ERR_OK)
		return -1;
#endif
	return (int)l2t.spa;
}

static void
asic_delete_mcast_mask(const unsigned char* mcast_mac, u32 port_fid)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.fid = (u16)port_fid;
	rtl8370_setAsicL2LookupTb(&l2t);
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.cvid_fid = (u16)port_fid;
	rtl8367b_setAsicL2LookupTb(&l2t);
#endif
}

static void
asic_update_mcast_mask(u32 port_msk, const unsigned char* mcast_mac, u32 port_fid, int is_leave)
{
	rtk_api_ret_t retVal;
	u32 port_cpu_msk = (1u << LAN_PORT_CPU);
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.fid = (u16)port_fid;
	retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal == RT_ERR_OK)
	{
		u16 portmask_old = l2t.portmask;
		
		if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT)
			g_l2t_cache[l2t.address] = 1;
		
		l2t.fid = (u16)port_fid;
		l2t.ipmul = 0;
		l2t.static_bit = 1;
		l2t.portmask |= (u16)port_cpu_msk;
		if (!is_leave)
			l2t.portmask |= (u16)port_msk;
		else
			l2t.portmask &= ~port_msk;
		if (l2t.portmask == portmask_old)
			return;
		retVal = rtl8370_setAsicL2LookupTb(&l2t);
		if (retVal == RT_ERR_OK)
		{
			if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT)
				g_l2t_cache[l2t.address] = 1;
		}
	}
	else
	{
		l2t.fid = (u16)port_fid;
		l2t.ipmul = 0;
		l2t.static_bit = 1;
		l2t.portmask = (u16)port_cpu_msk;
		if (!is_leave)
			l2t.portmask |= (u16)port_msk;
		else
			l2t.portmask &= ~port_msk;
		retVal = rtl8370_setAsicL2LookupTb(&l2t);
		if (retVal == RT_ERR_OK)
		{
			if (l2t.address < RTK_MAX_NUM_OF_LEARN_LIMIT)
				g_l2t_cache[l2t.address] = 1;
		}
	}
#else
	rtl8367b_luttb l2t;

	memset(&l2t, 0, sizeof(l2t));
	memcpy(l2t.mac.octet, mcast_mac, ETHER_ADDR_LEN);
	l2t.cvid_fid  = (u16)port_fid;
	retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_MAC, &l2t);
	if (retVal == RT_ERR_OK)
	{
		u16 portmask_old = l2t.mbr;
		l2t.ivl_svl   = 0;
		l2t.cvid_fid  = (u16)port_fid;
		l2t.l3lookup  = 0;
		l2t.nosalearn = 1;
		l2t.mbr |= (u16)port_cpu_msk;
		if (!is_leave)
			l2t.mbr |= (u16)port_msk;
		else
			l2t.mbr &= ~port_msk;
		if (l2t.mbr == portmask_old)
			return;
		rtl8367b_setAsicL2LookupTb(&l2t);
	}
	else
	{
		l2t.ivl_svl   = 0;
		l2t.cvid_fid  = (u16)port_fid;
		l2t.l3lookup  = 0;
		l2t.nosalearn = 1;
		l2t.mbr = (u16)port_cpu_msk;
		if (!is_leave)
			l2t.mbr |= (u16)port_msk;
		else
			l2t.mbr &= ~port_msk;
		rtl8367b_setAsicL2LookupTb(&l2t);
	}
#endif
}

void igmp_init(void)
{
	spin_lock_init(&g_lut_lock);
#if defined(CONFIG_RTL8367_API_8370)
	memset(g_l2t_cache, 0, sizeof(g_l2t_cache));
#endif
}

void change_igmp_snooping_control(u32 igmp_snooping_enabled, int force_change)
{
	if (igmp_snooping_enabled) igmp_snooping_enabled = 1;

	if (g_igmp_snooping_enabled != igmp_snooping_enabled || force_change)
	{
		printk("%s - igmp snooping: %d\n", RTL8367_DEVNAME, igmp_snooping_enabled);
		
		g_igmp_snooping_enabled = igmp_snooping_enabled;
		if (!igmp_snooping_enabled)
			asic_dump_mcast_table(1);
	}
}

void asic_dump_mcast_table(int clear_mcast_table)
{
	rtk_api_ret_t retVal;
	int index = 0;
	u32 addr_id = 0;
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_luttb l2t;

	if (!clear_mcast_table)
		printk("%s - dump multicast LUT table:\n", RTL8367_DEVNAME);

	for (addr_id = 0; addr_id < RTK_MAX_NUM_OF_LEARN_LIMIT; addr_id++)
	{
		if (!g_l2t_cache[addr_id])
			continue;
		
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		spin_lock(&g_lut_lock);
		retVal = rtl8370_getAsicL2LookupTb(LUTREADMETHOD_ADDRESS, &l2t);
		spin_unlock(&g_lut_lock);
		if (retVal != RT_ERR_OK)
			continue;
		
		if (l2t.mac.octet[0] & 0x01)
		{
			index++;
			if (clear_mcast_table)
			{
				spin_lock(&g_lut_lock);
				asic_delete_mcast_mask(l2t.mac.octet, l2t.fid);
				spin_unlock(&g_lut_lock);
			}
			else
			{
				printk("  %d. %02X-%02X-%02X-%02X-%02X-%02X, portmask: 0x%04X, fid: %d\n", 
					index,
					l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
					l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
					l2t.portmask,
					l2t.fid
					);
			}
		}
	}

	if (clear_mcast_table) {
		spin_lock(&g_lut_lock);
		memset(g_l2t_cache, 0, sizeof(g_l2t_cache));
		spin_unlock(&g_lut_lock);
	}

#else
	rtl8367b_luttb l2t;

	if (!clear_mcast_table)
		printk("%s - dump multicast LUT table:\n", RTL8367_DEVNAME);

	for (;;)
	{
		if (addr_id > RTK_MAX_LUT_ADDR_ID)
			break;
		
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		spin_lock(&g_lut_lock);
		retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L2MC, &l2t);
		spin_unlock(&g_lut_lock);
		if (retVal != RT_ERR_OK)
			break;
		if (l2t.address < addr_id)
			break;
		
		index++;
		addr_id = l2t.address + 1;
		
		if (clear_mcast_table)
		{
			spin_lock(&g_lut_lock);
			asic_delete_mcast_mask(l2t.mac.octet, l2t.cvid_fid);
			spin_unlock(&g_lut_lock);
		}
		else
		{
			printk("  %d. %02X-%02X-%02X-%02X-%02X-%02X, addr_id: %d, portmask: 0x%04X, fid: %d\n", 
				index,
				l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
				l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
				l2t.address,
				l2t.mbr,
				l2t.cvid_fid
				);
		}
	}
#endif
}

void rtl8367_mcast_group_event(const unsigned char *mac_src, const unsigned char *mac_dst, const char *dev_name, int is_leave)
{
	int port_src_idx;
	u32 port_fid, port_src_mask, lan_grp_mask;

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

	port_fid = 1;
#if defined(EXT_PORT_INIC)
	if (strcmp(dev_name, "eth2.3") == 0)
		port_fid = INIC_GUEST_FID;
#endif

	/* get port_id by unicast source MAC in l2 lookup table */
	spin_lock(&g_lut_lock);
	port_src_idx = asic_find_port_with_ucast_mac(mac_src, port_fid);
	spin_unlock(&g_lut_lock);

	if (port_src_idx < 0)
		return;

	port_src_mask = (1u << (u32)port_src_idx);

	lan_grp_mask = get_phy_ports_mask_lan(0);
#if defined(EXT_PORT_INIC)
	lan_grp_mask |= (1u << EXT_PORT_INIC);
#endif

	if (!(port_src_mask & lan_grp_mask))
		return;

	/* update multicast MAC port mask in l2 lookup table */
	spin_lock(&g_lut_lock);
	asic_update_mcast_mask(port_src_mask, mac_dst, port_fid, is_leave);
	spin_unlock(&g_lut_lock);
}

EXPORT_SYMBOL(rtl8367_mcast_group_event);

