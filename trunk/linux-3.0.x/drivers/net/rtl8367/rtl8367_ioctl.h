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

#ifndef __RTL8367_IOCTL_H__
#define __RTL8367_IOCTL_H__

#define RTL8367_DEVPATH				"/dev/rtl8367"
#define RTL8367_DEVMAJOR			(206)

#define RTL8367_IOCTL_CMD_LENGTH_BITS		(8)

/////////////////////////////////////////////////
// SWITCH STATUS
/////////////////////////////////////////////////

#define RTL8367_IOCTL_STATUS_LINK_PORT		10
#define RTL8367_IOCTL_STATUS_LINK_PORTS_WAN	11
#define RTL8367_IOCTL_STATUS_LINK_PORTS_LAN	12
#define RTL8367_IOCTL_STATUS_LINK_CHANGED	17

#define RTL8367_IOCTL_STATUS_SPEED_PORT		20

#define RTL8367_IOCTL_STATUS_BYTES_PORT		26

#define RTL8367_IOCTL_STATUS_MIB_PORT		30
#define RTL8367_IOCTL_STATUS_MIB_RESET_ALL	38

/////////////////////////////////////////////////
// INIT CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_RESET_ASIC		40
#define RTL8367_IOCTL_PORTS_POWER		41
#define RTL8367_IOCTL_PORTS_WAN_LAN_POWER	42
#define RTL8367_IOCTL_MAC_TABLE_CLEAR		43

/////////////////////////////////////////////////
// BRIDGE CONTROL (WAN/LAN HW ISOLATION)
/////////////////////////////////////////////////

#define RTL8367_IOCTL_BRIDGE_MODE		50
#define RTL8367_IOCTL_ISOLATE_INIC		51
#define RTL8367_IOCTL_DISABLE_INIC		52
#define RTL8367_IOCTL_PORT_FORWARD_MASK		55

/////////////////////////////////////////////////
// VLAN CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_VLAN_RESET_TABLE		60
#define RTL8367_IOCTL_VLAN_PVID_WAN_GET		61
#define RTL8367_IOCTL_VLAN_ACCEPT_PORT_MODE	62
#define RTL8367_IOCTL_VLAN_CREATE_PORT_VID	63
#define RTL8367_IOCTL_VLAN_CREATE_ENTRY		64
#define RTL8367_IOCTL_VLAN_RULE_SET		65

/////////////////////////////////////////////////
// USER CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_STORM_UNICAST_UNK		70
#define RTL8367_IOCTL_STORM_MULTICAST_UNK	71
#define RTL8367_IOCTL_STORM_MULTICAST		72
#define RTL8367_IOCTL_STORM_BROADCAST		73

#define RTL8367_IOCTL_JUMBO_FRAMES		75
#define RTL8367_IOCTL_GREEN_ETHERNET		76
#define RTL8367_IOCTL_EEE_LPI			77

#define RTL8367_IOCTL_IGMP_SNOOPING		78
#define RTL8367_IOCTL_IGMP_STATIC_PORTS		79

#define RTL8367_IOCTL_LED_MODE_GROUP0		80
#define RTL8367_IOCTL_LED_MODE_GROUP1		81
#define RTL8367_IOCTL_LED_MODE_GROUP2		82

#define RTL8367_IOCTL_SPEED_PORT		90

/////////////////////////////////////////////////
// DEBUG CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_RGMII_DELAY_RX		100
#define RTL8367_IOCTL_RGMII_DELAY_TX		101

#define RTL8367_IOCTL_MCAST_LUT_DUMP		110
#define RTL8367_IOCTL_UCAST_MAC_DUMP		111
#define RTL8367_IOCTL_ISOLATION_DUMP		112

#define RTL8367_IOCTL_REG_GET			120
#define RTL8367_IOCTL_REG_SET			121
#define RTL8367_IOCTL_PHY_REG_GET		122
#define RTL8367_IOCTL_PHY_REG_SET		123

// *** VALUES DEFINITION ***

/////////////////////////////////////////////////
// MAGIC
/////////////////////////////////////////////////

#define SWAPI_MAGIC_RESET_ASIC			(0x25252525)

/////////////////////////////////////////////////
// HW INDEPENDENT PORT ID
/////////////////////////////////////////////////

#define SWAPI_PORT_WAN				0
#define SWAPI_PORT_LAN1				1
#define SWAPI_PORT_LAN2				2
#define SWAPI_PORT_LAN3				3
#define SWAPI_PORT_LAN4				4
#define SWAPI_PORT_LAN5				5
#define SWAPI_PORT_LAN6				6
#define SWAPI_PORT_LAN7				7
#define SWAPI_PORT_CPU_INIC			13
#define SWAPI_PORT_CPU_LAN			14
#define SWAPI_PORT_CPU_WAN			15

/////////////////////////////////////////////////
// HW INDEPENDENT PORT MASK
/////////////////////////////////////////////////

#define SWAPI_PORTMASK_WAN			(1u<<SWAPI_PORT_WAN)
#define SWAPI_PORTMASK_LAN1			(1u<<SWAPI_PORT_LAN1)
#define SWAPI_PORTMASK_LAN2			(1u<<SWAPI_PORT_LAN2)
#define SWAPI_PORTMASK_LAN3			(1u<<SWAPI_PORT_LAN3)
#define SWAPI_PORTMASK_LAN4			(1u<<SWAPI_PORT_LAN4)
#define SWAPI_PORTMASK_LAN5			(1u<<SWAPI_PORT_LAN5)
#define SWAPI_PORTMASK_LAN6			(1u<<SWAPI_PORT_LAN6)
#define SWAPI_PORTMASK_LAN7			(1u<<SWAPI_PORT_LAN7)
#define SWAPI_PORTMASK_CPU_INIC			(1u<<SWAPI_PORT_CPU_INIC)
#define SWAPI_PORTMASK_CPU_LAN			(1u<<SWAPI_PORT_CPU_LAN)
#define SWAPI_PORTMASK_CPU_WAN			(1u<<SWAPI_PORT_CPU_WAN)

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
///////////////////////////////////////////
//     CPU_EXTIF_0           CPU_EXTIF_1
//     |         |               |
//    WAN <--> BWAN(LANX)    LAN(others)
///////////////////////////////////////////

#define SWAPI_WAN_BWAN_ISOLATION_FROM_CPU	(1)
///////////////////////////////////////////
//     CPU_EXTIF_0           CPU_EXTIF_1
//     |                         |
//    WAN <--> BWAN(LANX)    LAN(others)
///////////////////////////////////////////

#define SWAPI_WAN_BWAN_ISOLATION_BETWEEN	(2)
///////////////////////////////////////////
//     CPU_EXTIF_0           CPU_EXTIF_1
//     |         |               |
//    WAN      BWAN(LANX)    LAN(others)
///////////////////////////////////////////

/////////////////////////////////////////////////
// VLAN MODES
/////////////////////////////////////////////////

#define SWAPI_VLAN_ACCEPT_FRAMES_ALL		0
#define SWAPI_VLAN_ACCEPT_FRAMES_TAG_ONLY	1
#define SWAPI_VLAN_ACCEPT_FRAMES_UNTAG_ONLY	2

#define SWAPI_VLAN_RULE_WAN_INET		0
#define SWAPI_VLAN_RULE_WAN_IPTV		1
#define SWAPI_VLAN_RULE_WAN_LAN1		2
#define SWAPI_VLAN_RULE_WAN_LAN2		3
#define SWAPI_VLAN_RULE_WAN_LAN3		4
#define SWAPI_VLAN_RULE_WAN_LAN4		5
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
#define SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF	(15)

#define SWAPI_LINK_FLOW_CONTROL_TX_RX		(0)
#define SWAPI_LINK_FLOW_CONTROL_TX_ASYNC	(1)
#define SWAPI_LINK_FLOW_CONTROL_DISABLE		(2)

/////////////////////////////////////////////////
// LED MODES
/////////////////////////////////////////////////

#define SWAPI_LED_PHYMODE_1000_ACT		(0)
#define SWAPI_LED_PHYMODE_100_ACT		(1)
#define SWAPI_LED_PHYMODE_10_ACT		(2)
#define SWAPI_LED_PHYMODE_100_10_ACT		(3)
#define SWAPI_LED_PHYMODE_1000			(4)
#define SWAPI_LED_PHYMODE_100			(5)
#define SWAPI_LED_PHYMODE_10			(6)
#define SWAPI_LED_LINK_ACT			(7)
#define SWAPI_LED_LINK_ACT_RX			(8)
#define SWAPI_LED_LINK_ACT_TX			(9)
#define SWAPI_LED_DUPLEX_COLLISION		(10)
#define SWAPI_LED_OFF				(11)

/////////////////////////////////////////////////

typedef struct port_bytes_s
{
	uint64_t RX;
	uint64_t TX;
} port_bytes_t;

#endif

