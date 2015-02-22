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
// GPIO CONTROL (ATOMIC WITH SMI)
/////////////////////////////////////////////////

#define RTL8367_IOCTL_GPIO_MODE_SET		0
#define RTL8367_IOCTL_GPIO_MODE_SET_BIT		1
#define RTL8367_IOCTL_GPIO_MODE_GET		2
#define RTL8367_IOCTL_GPIO_PIN_SET_DIRECTION	3
#define RTL8367_IOCTL_GPIO_PIN_SET_VAL		4
#define RTL8367_IOCTL_GPIO_PIN_GET_VAL		5

/////////////////////////////////////////////////
// SWITCH STATUS
/////////////////////////////////////////////////

#define RTL8367_IOCTL_STATUS_LINK_PORT_WAN	10
#define RTL8367_IOCTL_STATUS_LINK_PORTS_WAN	11
#define RTL8367_IOCTL_STATUS_LINK_PORTS_LAN	12
#define RTL8367_IOCTL_STATUS_LINK_PORT_LAN1	13
#define RTL8367_IOCTL_STATUS_LINK_PORT_LAN2	14
#define RTL8367_IOCTL_STATUS_LINK_PORT_LAN3	15
#define RTL8367_IOCTL_STATUS_LINK_PORT_LAN4	16
#define RTL8367_IOCTL_STATUS_LINK_CHANGED	17

#define RTL8367_IOCTL_STATUS_SPEED_PORT_WAN	20
#define RTL8367_IOCTL_STATUS_SPEED_PORT_LAN1	21
#define RTL8367_IOCTL_STATUS_SPEED_PORT_LAN2	22
#define RTL8367_IOCTL_STATUS_SPEED_PORT_LAN3	23
#define RTL8367_IOCTL_STATUS_SPEED_PORT_LAN4	24

#define RTL8367_IOCTL_STATUS_CNT_PORT_WAN	30
#define RTL8367_IOCTL_STATUS_CNT_PORT_LAN1	31
#define RTL8367_IOCTL_STATUS_CNT_PORT_LAN2	32
#define RTL8367_IOCTL_STATUS_CNT_PORT_LAN3	33
#define RTL8367_IOCTL_STATUS_CNT_PORT_LAN4	34
#define RTL8367_IOCTL_STATUS_CNT_PORT_CPU_WAN	35
#define RTL8367_IOCTL_STATUS_CNT_PORT_CPU_LAN	36
#define RTL8367_IOCTL_STATUS_CNT_PORT_INIC	37
#define RTL8367_IOCTL_STATUS_CNT_RESET_ALL	38

/////////////////////////////////////////////////
// INIT CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_RESET_ASIC		40
#define RTL8367_IOCTL_PORTS_POWER		41
#define RTL8367_IOCTL_PORTS_WAN_LAN_POWER	42

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
#define RTL8367_IOCTL_VLAN_INGRESS_MODE		61
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
#define RTL8367_IOCTL_IGMP_STATIC_PORTS		77
#define RTL8367_IOCTL_IGMP_SNOOPING		78
#define RTL8367_IOCTL_IGMP_RESET		79

#define RTL8367_IOCTL_LED_MODE_GROUP0		80
#define RTL8367_IOCTL_LED_MODE_GROUP1		81
#define RTL8367_IOCTL_LED_MODE_GROUP2		82

#define RTL8367_IOCTL_SPEED_PORT_WAN		90
#define RTL8367_IOCTL_SPEED_PORT_LAN1		91
#define RTL8367_IOCTL_SPEED_PORT_LAN2		92
#define RTL8367_IOCTL_SPEED_PORT_LAN3		93
#define RTL8367_IOCTL_SPEED_PORT_LAN4		94

/////////////////////////////////////////////////
// DEBUG CONTROL
/////////////////////////////////////////////////

#define RTL8367_IOCTL_RGMII_DELAY_RX		100
#define RTL8367_IOCTL_RGMII_DELAY_TX		101

#define RTL8367_IOCTL_MCAST_LUT_DUMP		110
#define RTL8367_IOCTL_ISOLATION_DUMP		111

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
#define SWAPI_PORTMASK_INIC			(1<<7)

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

#endif

