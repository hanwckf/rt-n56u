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

#ifndef __MTK_ESW_DEF_H__
#define __MTK_ESW_DEF_H__

////////////////////////////////////////////////////////////////////////////////////

#define MTK_ESW_DEVNAME			"mtk_esw"

#define WAN_PORT_X			CONFIG_RAETH_ESW_PORT_WAN	/* 8P8C WAN  */
#define LAN_PORT_1			CONFIG_RAETH_ESW_PORT_LAN1	/* 8P8C LAN1 */
#define LAN_PORT_2			CONFIG_RAETH_ESW_PORT_LAN2	/* 8P8C LAN2 */
#define LAN_PORT_3			CONFIG_RAETH_ESW_PORT_LAN3	/* 8P8C LAN3 */
#define LAN_PORT_4			CONFIG_RAETH_ESW_PORT_LAN4	/* 8P8C LAN4 */

#define LAN_PORT_CPU			6
#define WAN_PORT_CPU			6
#define ESW_PORT_CPU			6
#if defined (CONFIG_RALINK_MT7620)
#define ESW_PORT_PPE			7
#define ESW_PORT_ID_MAX			7
#else
#undef  ESW_PORT_PPE
#define ESW_PORT_ID_MAX			6
#endif

#define ESW_PHY_ID_MAX			4
#define ESW_VLAN_ID_MAX			15

////////////////////////////////////////////////////////////////////////////////////

#define ESW_DEFAULT_JUMBO_FRAMES	0
#define ESW_DEFAULT_STORM_RATE		0
#define ESW_DEFAULT_IGMP_SNOOPING	1

////////////////////////////////////////////////////////////////////////////////////

#define MIN_EXT_VLAN_VID		3
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

typedef struct bwan_member_s
{
	u8 bwan:1;
	u8 rule:7;
} bwan_member_t;

typedef struct pvlan_member_s
{
	u32 pvid:12;
	u32 prio:3;
	u32 tagg:1;
	u32 swap:1;
	u32 unused:15;
} pvlan_member_t;

typedef struct vlan_entry_s
{
	u32 valid:1;
	u32 fid:3;
	u32 cvid:12;
	u32 svid:12;
	u32 unused1:4;
	u32 port_member:8;
	u32 unused2:24;
} vlan_entry_t;

typedef struct mib_threshold_s
{
	u32 tx_goct_thr;
	u32 rx_goct_thr;
	u16 tx_good_thr;
	u16 rx_good_thr;
	u16 tx_bad_thr;
	u16 rx_bad_thr;
	u16 rx_arl_drop_thr;
	u16 rx_filter_thr;

} mib_threshold_t;

typedef union _ULARGE_INTEGER {
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	} u;
	uint64_t QuadPart;
} ULARGE_INTEGER;

////////////////////////////////////////////////////////////////////////////////////

#endif

