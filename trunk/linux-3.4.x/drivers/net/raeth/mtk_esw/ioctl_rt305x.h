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

#ifndef __IOCTL_RT305X_H__
#define __IOCTL_RT305X_H__

////////////////////////////////////////////////////////////////////////////////////

#define WAN_PORT_X			CONFIG_RAETH_ESW_PORT_WAN
#define LAN_PORT_1			CONFIG_RAETH_ESW_PORT_LAN1	/* 8P8C LAN1 */
#define LAN_PORT_2			CONFIG_RAETH_ESW_PORT_LAN2	/* 8P8C LAN2 */
#define LAN_PORT_3			CONFIG_RAETH_ESW_PORT_LAN3	/* 8P8C LAN3 */
#define LAN_PORT_4			CONFIG_RAETH_ESW_PORT_LAN4	/* 8P8C LAN4 */

#define MASK_WAN_PORT_X			(1u << WAN_PORT_X)
#define MASK_LAN_PORT_1			(1u << LAN_PORT_1)
#define MASK_LAN_PORT_2			(1u << LAN_PORT_2)
#define MASK_LAN_PORT_3			(1u << LAN_PORT_3)
#define MASK_LAN_PORT_4			(1u << LAN_PORT_4)

#if defined (CONFIG_RAETH_ESW_PORT_LAN5) && (CONFIG_RAETH_ESW_PORT_LAN5 >= 0)
#define LAN_PORT_5			CONFIG_RAETH_ESW_PORT_LAN5
#define MASK_LAN_PORT_5			(1u << LAN_PORT_5)
#else
#define MASK_LAN_PORT_5			0
#endif

#define ESW_PORT_CPU			6
#define MASK_ESW_PORT_CPU		(1u << ESW_PORT_CPU)

#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
#define ESW_EPHY_ID_MAX			5
#define ESW_MASK_EXCLUDE		0
#else
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#endif

////////////////////////////////////////////////////////////////////////////////////

#define ESW_DEFAULT_EEE_LPI		0
#define ESW_DEFAULT_STORM_RATE		0

////////////////////////////////////////////////////////////////////////////////////

#define MIN_EXT_VLAN_VID		2
#define ESW_PRINT_LINK_ALL		0	/* printk only WAN link changed */

////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	u8 bwan:1;
	u8 rule:7;
} bwan_member_t;

typedef struct
{
	u16 pvid:12;
	u16 prio:3;
	u16 tagg:1;
} pvlan_member_t;

typedef struct
{
	u32 valid:1;
	u32 cvid:12;
	u32 unused:3;
	u32 port_member:8;
	u32 port_untag:8;
} vlan_entry_t;

////////////////////////////////////////////////////////////////////////////////////

typedef struct esw_mib_counters_s
{
	uint32_t TxGoodFrames;
	uint32_t TxBadFrames;
	uint32_t RxGoodFrames;
	uint32_t RxBadFrames;
} esw_mib_counters_t;

////////////////////////////////////////////////////////////////////////////////////

u32 get_ports_mask_lan(u32 include_cpu);
void esw_igmp_flood_to_cpu(int flood_to_cpu);

////////////////////////////////////////////////////////////////////////////////////

#endif

