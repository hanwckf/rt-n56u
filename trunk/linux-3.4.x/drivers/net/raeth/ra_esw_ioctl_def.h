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

#ifndef __MTK_ESW_IOCTL_DEF_H__
#define __MTK_ESW_IOCTL_DEF_H__

////////////////////////////////////////////////////////////////////////////////////

#define MTK_ESW_DEVNAME			"mtk_esw"

#define LAN_PORT_1			CONFIG_RAETH_ESW_PORT_LAN1	/* 8P8C LAN1 */
#define LAN_PORT_2			CONFIG_RAETH_ESW_PORT_LAN2	/* 8P8C LAN2 */
#define LAN_PORT_3			CONFIG_RAETH_ESW_PORT_LAN3	/* 8P8C LAN3 */
#define LAN_PORT_4			CONFIG_RAETH_ESW_PORT_LAN4	/* 8P8C LAN4 */

#define ESW_PORT_CPU			6
#define LAN_PORT_CPU			6

#if defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P0)
#define WAN_PORT_PHY			0	/* P0 PHY */
#define WAN_PORT_MAC			0	/* fake */
#define WAN_PORT_CPU			5	/* fake */
#define ESW_MAC_ID_MAX			4
#define ESW_PHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		((1<<5)|(1<<0))	/* P5/P0 excluded */
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
#define WAN_PORT_PHY			4	/* P4 PHY */
#define WAN_PORT_MAC			4	/* fake */
#define WAN_PORT_CPU			5	/* fake */
#define ESW_MAC_ID_MAX			4
#define ESW_PHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		((1<<5)|(1<<4))	/* P5/P4 excluded */
#elif defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
#define WAN_PORT_PHY			CONFIG_RAETH_ESW_PORT_WAN	/* 8P8C WAN */
#define WAN_PORT_MAC			WAN_PORT_PHY
#define WAN_PORT_CPU			5	/* P5 = CPU WAN */
#define ESW_MAC_ID_MAX			4
#define ESW_PHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		0
#else
#define WAN_PORT_PHY			CONFIG_RAETH_ESW_PORT_WAN	/* 8P8C WAN */
#define WAN_PORT_MAC			WAN_PORT_PHY
#define WAN_PORT_CPU			6	/* P6 = CPU LAN + WAN */
#define ESW_MAC_ID_MAX			4
#if defined (CONFIG_GE2_RGMII_AN)
#define ESW_PHY_ID_MAX			5
#else
#define ESW_PHY_ID_MAX			4
#endif
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#endif
#else /* !CONFIG_MT7530_GSW */
#define WAN_PORT_PHY			CONFIG_RAETH_ESW_PORT_WAN	/* 8P8C WAN */
#define WAN_PORT_MAC			WAN_PORT_PHY
#define WAN_PORT_CPU			6	/* P6 = CPU LAN + WAN */
#if defined (CONFIG_RAETH_HAS_PORT5)
#define ESW_MAC_ID_MAX			5
#define ESW_PHY_ID_MAX			5
#if (LAN_PORT_1 != 4) && (LAN_PORT_2 != 4) && (LAN_PORT_3 != 4) && (LAN_PORT_4 != 4) && (WAN_PORT_PHY != 4)
#define ESW_MASK_EXCLUDE		(1<<4)	/* P4 excluded */
#else
#define ESW_MASK_EXCLUDE		0
#endif
#else
#define ESW_MAC_ID_MAX			4
#define ESW_PHY_ID_MAX			4
#define ESW_MASK_EXCLUDE		(1<<5)	/* P5 excluded */
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////

#define ESW_DEFAULT_JUMBO_FRAMES	0
#define ESW_DEFAULT_GREEN_ETHERNET	0
#define ESW_DEFAULT_STORM_RATE		0
#define ESW_DEFAULT_IGMP_SNOOPING	1

////////////////////////////////////////////////////////////////////////////////////

#define MIN_EXT_VLAN_VID		2
#define ESW_USE_IVL_MODE		1	/* always use IVL instead of SVL */
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
	u32 pvid:12;
	u32 prio:3;
	u32 tagg:1;
	u32 swap:1;
	u32 unused:15;
} pvlan_member_t;

typedef struct
{
	u32 valid:1;
	u32 fid:3;
	u32 cvid:12;
	u32 svid:12;
	u32 unused1:4;
	u32 port_member:8;
	u32 unused2:24;
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
	uint32_t TxCollision;
	uint32_t TxCRCError;
	uint64_t RxGoodOctets;
	uint32_t RxUcastFrames;
	uint32_t RxMcastFrames;
	uint32_t RxBcastFrames;
	uint32_t RxDropFrames;
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

/*
	MT7530 MIB (docs needed)

	"Tx Drop Packet      :" 0x4000
	"Tx CRC Error        :" 0x4004
	"Tx Unicast Packet   :" 0x4008
	"Tx Multicast Packet :" 0x400C
	"Tx Broadcast Packet :" 0x4010
	"Tx Collision Event  :" 0x4014
	"Tx Pause Packet     :" 0x402C
	"Tx ???              :" 0x4030
	"Tx ???              :" 0x4034
	"Tx ???              :" 0x4038
	"Tx ???              :" 0x403C
	"Tx ???              :" 0x4040
	"Tx ???              :" 0x4044
	"Tx Good Octets Lo   :" 0x4048
	"Tx Good Octets Hi   :" 0x404C

	"Rx Drop Packet      :" 0x4060
	"Rx Filtering Packet :" 0x4064
	"Rx Unicast Packet   :" 0x4068
	"Rx Multicast Packet :" 0x406C
	"Rx Broadcast Packet :" 0x4070
	"Rx Alignment Error  :" 0x4074
	"Rx CRC Error        :" 0x4078
	"Rx Undersize Error  :" 0x407C
	"Rx Fragment Error   :" 0x4080
	"Rx Oversize Error   :" 0x4084
	"Rx Jabber Error     :" 0x4088
	"Rx Pause Packet     :" 0x408C
	"Rx ???              :" 0x4090
	"Rx ???              :" 0x4094
	"Rx ???              :" 0x4098
	"Rx ???              :" 0x409C
	"Rx ???              :" 0x40A0
	"Rx ???              :" 0x40A4
	"Rx Good Octets Lo   :" 0x40A8
	"Rx Good Octets Hi   :" 0x40AC
*/

#endif

