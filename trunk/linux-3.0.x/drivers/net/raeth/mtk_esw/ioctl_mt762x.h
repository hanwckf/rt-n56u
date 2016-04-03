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

#ifndef __IOCTL_MT762X_H__
#define __IOCTL_MT762X_H__

////////////////////////////////////////////////////////////////////////////////////

#define LAN_PORT_1			CONFIG_RAETH_ESW_PORT_LAN1
#define LAN_PORT_2			CONFIG_RAETH_ESW_PORT_LAN2
#define LAN_PORT_3			CONFIG_RAETH_ESW_PORT_LAN3
#define LAN_PORT_4			CONFIG_RAETH_ESW_PORT_LAN4

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

#define MASK_LAN_PORTS_ALL		(MASK_LAN_PORT_1|MASK_LAN_PORT_2|MASK_LAN_PORT_3|MASK_LAN_PORT_4|MASK_LAN_PORT_5)

#define ESW_PORT_CPU			6
#define LAN_PORT_CPU			6
#define MASK_LAN_PORT_CPU		(1u << LAN_PORT_CPU)

#if defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
#define WAN_PORT_X			CONFIG_RAETH_ESW_PORT_WAN
#define WAN_PORT_CPU			5	/* P5 = CPU WAN */
#define MASK_WAN_PORT_X			(1u << WAN_PORT_X)
#define MASK_WAN_PORT_CPU		(1u << WAN_PORT_CPU)
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		0
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P0)
#define WAN_PORT_X			0	/* P0 PHY */
#define WAN_PORT_CPU			5	/* fake */
#define MASK_WAN_PORT_X			0
#define MASK_WAN_PORT_CPU		0
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		((1<<5)|(1<<0))	/* P5/P0 excluded */
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
#define WAN_PORT_X			4	/* P4 PHY */
#define WAN_PORT_CPU			5	/* fake */
#define MASK_WAN_PORT_X			0
#define MASK_WAN_PORT_CPU		0
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		((1<<5)|(1<<4))	/* P5/P4 excluded */
#elif defined (CONFIG_GE2_RGMII_AN)
#define WAN_PORT_X			5	/* External PHY */
#define WAN_PORT_CPU			5	/* fake */
#define MASK_WAN_PORT_X			0
#define MASK_WAN_PORT_CPU		0
#define ESW_EPHY_ID_MAX			5
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#else
#define WAN_PORT_X			CONFIG_RAETH_ESW_PORT_WAN
#define WAN_PORT_CPU			6	/* P6 = CPU LAN + WAN */
#define MASK_WAN_PORT_X			(1u << WAN_PORT_X)
#define MASK_WAN_PORT_CPU		(1u << WAN_PORT_CPU)
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#endif
#else /* !CONFIG_MT7530_GSW */
#define WAN_PORT_X			CONFIG_RAETH_ESW_PORT_WAN
#define WAN_PORT_CPU			6	/* P6 = CPU LAN + WAN */
#define MASK_WAN_PORT_X			(1u << WAN_PORT_X)
#define MASK_WAN_PORT_CPU		(1u << WAN_PORT_CPU)
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
#define ESW_EPHY_ID_MAX			5
#define ESW_MASK_EXCLUDE		0
#else
#define ESW_EPHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#endif
#endif

#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P0) || \
    defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P4) || \
    defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
#define MT7530_P5_ENABLED
#endif

#if defined (MT7530_P5_ENABLED) || defined (CONFIG_RAETH_GMAC2)
#define MT7530_P6_UNTAGGED
#endif

#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_RGMII_AN) || \
    defined (CONFIG_GE2_INTERNAL_GPHY_P4)
#define RAETH_GE2_MAC_TO_GPHY
#endif

////////////////////////////////////////////////////////////////////////////////////

#define ESW_DEFAULT_JUMBO_FRAMES	0
#define ESW_DEFAULT_EEE_LPI		0
#define ESW_DEFAULT_STORM_RATE		0
#define ESW_DEFAULT_IGMP_SNOOPING	1

////////////////////////////////////////////////////////////////////////////////////

#define MIN_EXT_VLAN_VID		2
#define ESW_USE_IVL_MODE		1	/* always use IVL (instead of SVL) */
#define ESW_PRINT_LINK_ALL		0	/* printk only WAN link changed */

////////////////////////////////////////////////////////////////////////////////////

enum
{
	PVLAN_INGRESS_MODE_MATRIX   = 0x00,
	PVLAN_INGRESS_MODE_FALLBACK = 0x01,
	PVLAN_INGRESS_MODE_CHECK    = 0x02,
	PVLAN_INGRESS_MODE_SECURITY = 0x03
};

enum
{
	PVLAN_EGRESS_UNTAG = 0x00,
	PVLAN_EGRESS_SWAP  = 0x01,
	PVLAN_EGRESS_TAG   = 0x02,
	PVLAN_EGRESS_STACK = 0x03
};

enum
{
	PORT_ACCEPT_FRAMES_ALL      = 0x00,
	PORT_ACCEPT_FRAMES_TAGGED   = 0x01,
	PORT_ACCEPT_FRAMES_UNTAGGED = 0x02
};

enum
{
	PORT_ATTRIBUTE_USER         = 0x00,
	PORT_ATTRIBUTE_STACK        = 0x01,
	PORT_ATTRIBUTE_TRANSLATION  = 0x02,
	PORT_ATTRIBUTE_TRANSPARENT  = 0x03
};

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
	u32 fid:3;
	u32 cvid:12;
	u32 svid:12;
	u32 unused1:4;
	u32 port_member:8;
	u32 port_untag:8;
	u32 port_swap:8;
	u32 unused2:8;
} vlan_entry_t;

////////////////////////////////////////////////////////////////////////////////////

#if defined (CONFIG_MT7530_GSW)
typedef struct esw_mib_counters_s
{
	uint64_t TxGoodOctets;
	uint32_t TxUcastFrames;
	uint32_t TxMcastFrames;
	uint32_t TxBcastFrames;
	uint32_t TxDropFrames;
	uint32_t TxPauseFrames;
	uint32_t TxCollision;
	uint32_t TxCRCError;
	uint64_t RxGoodOctets;
	uint32_t RxUcastFrames;
	uint32_t RxMcastFrames;
	uint32_t RxBcastFrames;
	uint32_t RxDropFrames;
	uint32_t RxPauseFrames;
	uint32_t RxFilterFrames;
	uint32_t RxCRCError;
	uint32_t RxAligmentError;
} esw_mib_counters_t;
#else
typedef struct esw_mib_counters_s
{
	uint64_t TxGoodOctets;
	uint32_t TxGoodFrames;
	uint32_t TxBadOctets;
	uint32_t TxBadFrames;
	uint32_t TxDropFrames;
	uint64_t RxGoodOctets;
	uint32_t RxGoodFrames;
	uint32_t RxBadOctets;
	uint32_t RxBadFrames;
	uint32_t RxDropFramesFilter;
	uint32_t RxDropFramesErr;
} esw_mib_counters_t;
#endif

////////////////////////////////////////////////////////////////////////////////////

u32 get_ports_mask_lan(u32 include_cpu, int is_phy_id);
void esw_igmp_flood_to_cpu(int flood_to_cpu);

////////////////////////////////////////////////////////////////////////////////////

#endif

