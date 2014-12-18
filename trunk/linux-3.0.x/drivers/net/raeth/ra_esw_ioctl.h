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

#ifndef __MTK_ESW_IOCTL_H__
#define __MTK_ESW_IOCTL_H__

#define MTK_ESW_DEVPATH				"/dev/mtk_esw"
#define MTK_ESW_DEVMAJOR			(219)

#define MTK_ESW_IOCTL_CMD_LENGTH_BITS		(8)

/////////////////////////////////////////////////
// GPIO CONTROL
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_GPIO_MODE_SET		0
#define MTK_ESW_IOCTL_GPIO_MODE_SET_BIT		1
#define MTK_ESW_IOCTL_GPIO_MODE_GET		2
#define MTK_ESW_IOCTL_GPIO_PIN_SET_DIRECTION	3
#define MTK_ESW_IOCTL_GPIO_PIN_SET_VAL		4
#define MTK_ESW_IOCTL_GPIO_PIN_GET_VAL		5

/////////////////////////////////////////////////
// SWITCH STATUS
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN	10
#define MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN	11
#define MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN	12
#define MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN1	13
#define MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN2	14
#define MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN3	15
#define MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN4	16
#define MTK_ESW_IOCTL_STATUS_LINK_CHANGED	17

#define MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN	20
#define MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1	21
#define MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2	22
#define MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3	23
#define MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4	24

#define MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN	30
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1	31
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2	32
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3	33
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4	34
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_WAN	35
#define MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_LAN	36

/////////////////////////////////////////////////
// INIT CONTROL
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_RESET_SWITCH		40
#define MTK_ESW_IOCTL_PORT_POWER		41
#define MTK_ESW_IOCTL_MAC_TABLE_CLEAR		42

/////////////////////////////////////////////////
// BRIDGE CONTROL (WAN/LAN HW ISOLATION)
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_BRIDGE_MODE		50

/////////////////////////////////////////////////
// VLAN CONTROL
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_VLAN_RESET_TABLE		60
#define MTK_ESW_IOCTL_VLAN_ACCEPT_PORT_MODE	62
#define MTK_ESW_IOCTL_VLAN_CREATE_PORT_VID	63
#define MTK_ESW_IOCTL_VLAN_CREATE_ENTRY		64
#define MTK_ESW_IOCTL_VLAN_RULE_SET		65

/////////////////////////////////////////////////
// USER CONTROL
/////////////////////////////////////////////////

#define MTK_ESW_IOCTL_STORM_MULTICAST_UNK	71

#define MTK_ESW_IOCTL_JUMBO_FRAMES		75
#define MTK_ESW_IOCTL_GREEN_ETHERNET		76
#define MTK_ESW_IOCTL_IGMP_SNOOPING		78
#define MTK_ESW_IOCTL_IGMP_RESET		79

#define MTK_ESW_IOCTL_LED_MODE			80

#define MTK_ESW_IOCTL_SPEED_PORT_WAN		90
#define MTK_ESW_IOCTL_SPEED_PORT_LAN1		91
#define MTK_ESW_IOCTL_SPEED_PORT_LAN2		92
#define MTK_ESW_IOCTL_SPEED_PORT_LAN3		93
#define MTK_ESW_IOCTL_SPEED_PORT_LAN4		94

// *** VALUES DEFINITION ***

/////////////////////////////////////////////////
// MAGIC
/////////////////////////////////////////////////

#define SWAPI_MAGIC_RESET_ASIC			(0x25252525)

/////////////////////////////////////////////////
// HW INDEPENDED PORT MASK
/////////////////////////////////////////////////

#define SWAPI_PORTMASK_LAN1			(1<<0)
#define SWAPI_PORTMASK_LAN2			(1<<1)
#define SWAPI_PORTMASK_LAN3			(1<<2)
#define SWAPI_PORTMASK_LAN4			(1<<3)
#define SWAPI_PORTMASK_WAN			(1<<4)
#define SWAPI_PORTMASK_CPU_LAN			(1<<5)
#define SWAPI_PORTMASK_CPU_WAN			(1<<6)

/////////////////////////////////////////////////
// BRIDGE MODES
/////////////////////////////////////////////////

#define SWAPI_WAN_BRIDGE_DISABLE		(0)
#define SWAPI_WAN_BRIDGE_LAN1			(1)
#define SWAPI_WAN_BRIDGE_LAN2			(2)
#define SWAPI_WAN_BRIDGE_LAN3			(3)
#define SWAPI_WAN_BRIDGE_LAN4			(4)
#define SWAPI_WAN_BRIDGE_LAN3_LAN4		(5)
#define SWAPI_WAN_BRIDGE_LAN1_LAN2		(6)
#define SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3		(7)
#define SWAPI_WAN_BRIDGE_DISABLE_WAN		(8)
#define SWAPI_WAN_BRIDGE_NUM			(SWAPI_WAN_BRIDGE_DISABLE_WAN+1)

/////////////////////////////////////////////////
// BRIDGE WAN ISOLATION MODES
/////////////////////////////////////////////////

#define SWAPI_WAN_BWAN_ISOLATION_NONE		(0)
#define SWAPI_WAN_BWAN_ISOLATION_FROM_CPU	(1)
#define SWAPI_WAN_BWAN_ISOLATION_BETWEEN	(2)

/////////////////////////////////////////////////
// VLAN MODES
/////////////////////////////////////////////////

#define SWAPI_VLAN_ACCEPT_FRAMES_ALL		(0)
#define SWAPI_VLAN_ACCEPT_FRAMES_TAG_ONLY	(1)
#define SWAPI_VLAN_ACCEPT_FRAMES_UNTAG_ONLY	(2)

#define SWAPI_VLAN_RULE_WAN_INET		(0)
#define SWAPI_VLAN_RULE_WAN_IPTV		(1)
#define SWAPI_VLAN_RULE_WAN_LAN1		(2)
#define SWAPI_VLAN_RULE_WAN_LAN2		(3)
#define SWAPI_VLAN_RULE_WAN_LAN3		(4)
#define SWAPI_VLAN_RULE_WAN_LAN4		(5)
#define SWAPI_VLAN_RULE_NUM			(SWAPI_VLAN_RULE_WAN_LAN4+1)

/////////////////////////////////////////////////
// LINK MODES
/////////////////////////////////////////////////

#define SWAPI_LINK_SPEED_MODE_AUTO		(0)
#define SWAPI_LINK_SPEED_MODE_AUTO_1000_FD	(1)
#define SWAPI_LINK_SPEED_MODE_AUTO_100_FD	(2)
#define SWAPI_LINK_SPEED_MODE_AUTO_100_HD	(3)
#define SWAPI_LINK_SPEED_MODE_AUTO_10_FD	(4)
#define SWAPI_LINK_SPEED_MODE_AUTO_10_HD	(5)
#define SWAPI_LINK_SPEED_MODE_FORCE_100_FD	(6)
#define SWAPI_LINK_SPEED_MODE_FORCE_100_HD	(7)
#define SWAPI_LINK_SPEED_MODE_FORCE_10_FD	(8)
#define SWAPI_LINK_SPEED_MODE_FORCE_10_HD	(9)

#define SWAPI_LINK_FLOW_CONTROL_TX_RX		(0)
#define SWAPI_LINK_FLOW_CONTROL_RX_ASYNC	(1)
#define SWAPI_LINK_FLOW_CONTROL_DISABLE		(2)

/////////////////////////////////////////////////
// LED MODES
/////////////////////////////////////////////////

#define SWAPI_LED_LINK_ACT			(7)
#define SWAPI_LED_OFF				(11)

/////////////////////////////////////////////////
// MIB_COUNTERS
/////////////////////////////////////////////////

typedef struct arl_mib_counters_s
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

} arl_mib_counters_t;


#endif

