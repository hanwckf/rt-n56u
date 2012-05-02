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
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef __RTL8367M_DRV_H__
#define __RTL8367M_DRV_H__


#define RTL8367M_DEVPATH			"/dev/rtl8367m"

#define RTL8367M_IOCTL_CMD_LENGTH_BITS		(8)

/////////////////////////////////////////////////
// GPIO CONTROL (ATOMIC WITH SMI)
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_GPIO_MODE_SET		0
#define RTL8367M_IOCTL_GPIO_MODE_SET_BIT	1
#define RTL8367M_IOCTL_GPIO_MODE_GET		2
#define RTL8367M_IOCTL_GPIO_PIN_SET_DIRECTION	3
#define RTL8367M_IOCTL_GPIO_PIN_SET_VAL		4
#define RTL8367M_IOCTL_GPIO_PIN_GET_VAL		5

/////////////////////////////////////////////////
// SWITCH STATUS
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_STATUS_LINK_PORT_WAN	10
#define RTL8367M_IOCTL_STATUS_LINK_PORTS_WAN	11
#define RTL8367M_IOCTL_STATUS_LINK_PORTS_LAN	12

#define RTL8367M_IOCTL_STATUS_SPEED_PORT_WAN	20
#define RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN1	21
#define RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN2	22
#define RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN3	23
#define RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN4	24

#define RTL8367M_IOCTL_STATUS_CNT_PORT_WAN	30
#define RTL8367M_IOCTL_STATUS_CNT_PORT_LAN1	31
#define RTL8367M_IOCTL_STATUS_CNT_PORT_LAN2	32
#define RTL8367M_IOCTL_STATUS_CNT_PORT_LAN3	33
#define RTL8367M_IOCTL_STATUS_CNT_PORT_LAN4	34
#define RTL8367M_IOCTL_STATUS_CNT_RESET_ALL	35

/////////////////////////////////////////////////
// INIT CONTROL
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_RESET_ASIC		40

/////////////////////////////////////////////////
// BRIDGE CONTROL (WAN/LAN HW ISOLATION)
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_BRIDGE_MODE		50

/////////////////////////////////////////////////
// VLAN CONTROL
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_VLAN_RESET_TABLE		60
#define RTL8367M_IOCTL_VLAN_INGRESS_MODE	61
#define RTL8367M_IOCTL_VLAN_ACCEPT_PORT_MODE	62
#define RTL8367M_IOCTL_VLAN_CREATE_PORT_VID	63

/////////////////////////////////////////////////
// USER CONTROL
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_STORM_UNICAST_UNK	70
#define RTL8367M_IOCTL_STORM_MULTICAST_UNK	71
#define RTL8367M_IOCTL_STORM_MULTICAST		72
#define RTL8367M_IOCTL_STORM_BROADCAST		73

#define RTL8367M_IOCTL_JUMBO_FRAMES		75
#define RTL8367M_IOCTL_GREEN_ETHERNET		76

#define RTL8367M_IOCTL_LED_MODE_GREEN		80
#define RTL8367M_IOCTL_LED_MODE_YELLOW		81

#define RTL8367M_IOCTL_SPEED_PORT_WAN		90
#define RTL8367M_IOCTL_SPEED_PORT_LAN1		91
#define RTL8367M_IOCTL_SPEED_PORT_LAN2		92
#define RTL8367M_IOCTL_SPEED_PORT_LAN3		93
#define RTL8367M_IOCTL_SPEED_PORT_LAN4		94

/////////////////////////////////////////////////
// DEBUG CONTROL
/////////////////////////////////////////////////

#define RTL8367M_IOCTL_RGMII_DELAY_RX		100



/////////////////////////////////////////////////
// MAGIC VALUES
/////////////////////////////////////////////////

#define RTL8367M_MAGIC_RESET_ASIC		(0x25252525)

/////////////////////////////////////////////////
// BRIDGE MODES
/////////////////////////////////////////////////

#define RTL8367M_WAN_BRIDGE_DISABLE		(0)
#define RTL8367M_WAN_BRIDGE_LAN1		(1)
#define RTL8367M_WAN_BRIDGE_LAN2		(2)
#define RTL8367M_WAN_BRIDGE_LAN3		(3)
#define RTL8367M_WAN_BRIDGE_LAN4		(4)
#define RTL8367M_WAN_BRIDGE_LAN3_LAN4		(5)
#define RTL8367M_WAN_BRIDGE_LAN1_LAN2		(6)
#define RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3	(7)

/////////////////////////////////////////////////
// BRIDGE WAN ISOLATION MODES
/////////////////////////////////////////////////

#define RTL8367M_WAN_BWAN_ISOLATION_NONE	(0)
///////////////////////////////////////////
//      CPU_GMAC2           CPU_GMAC1
//     |         |               |
//    WAN <--> BWAN(LANX)    LAN(others)
///////////////////////////////////////////

#define RTL8367M_WAN_BWAN_ISOLATION_FROM_CPU	(1)
///////////////////////////////////////////
//      CPU_GMAC2           CPU_GMAC1
//     |                         |
//    WAN <--> BWAN(LANX)    LAN(others)
///////////////////////////////////////////

#define RTL8367M_WAN_BWAN_ISOLATION_BETWEEN	(2)
///////////////////////////////////////////
//      CPU_GMAC2           CPU_GMAC1
//     |         |               |
//    WAN      BWAN(LANX)    LAN(others)
///////////////////////////////////////////

/////////////////////////////////////////////////
// VLAN MODES
/////////////////////////////////////////////////

#define RTL8367M_VLAN_INGRESS_FILTER_DISABLED	0
#define RTL8367M_VLAN_INGRESS_FILTER_ENABLED	1

#define RTL8367M_VLAN_ACCEPT_FRAMES_ALL		0
#define RTL8367M_VLAN_ACCEPT_FRAMES_TAG_ONLY	1
#define RTL8367M_VLAN_ACCEPT_FRAMES_UNTAG_ONLY	2

/////////////////////////////////////////////////
// LED MODES
/////////////////////////////////////////////////

#define RTL8367M_LED_PHYMODE_1000_ACT		(0)
#define RTL8367M_LED_PHYMODE_100_ACT		(1)
#define RTL8367M_LED_PHYMODE_10_ACT		(2)
#define RTL8367M_LED_PHYMODE_100_10_ACT		(3)
#define RTL8367M_LED_PHYMODE_1000		(4)
#define RTL8367M_LED_PHYMODE_100		(5)
#define RTL8367M_LED_PHYMODE_10			(6)
#define RTL8367M_LED_LINK_ACT			(7)
#define RTL8367M_LED_LINK_ACT_RX		(8)
#define RTL8367M_LED_LINK_ACT_TX		(9)
#define RTL8367M_LED_DUPLEX_COLLISION		(10)

/////////////////////////////////////////////////
// PHY PORTS ID
/////////////////////////////////////////////////

#define WAN_PORT_CPU				9	/* P9 -> RT3662 GMAC2 WAN */
#define LAN_PORT_CPU				8	/* P8 -> RT3662 GMAC1 LAN */
#define WAN_PORT_X				4	/* P4 -> RJ45 WAN  */
#define LAN_PORT_1				3	/* P3 -> RJ45 LAN1 */
#define LAN_PORT_2				2	/* P2 -> RJ45 LAN2 */
#define LAN_PORT_3				1	/* P1 -> RJ45 LAN3 */
#define LAN_PORT_4				0	/* P0 -> RJ45 LAN4 */


#endif

