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

#ifndef __RTL8367_DEF_H__
#define __RTL8367_DEF_H__

//#define RTL8367_DBG 1

#define RTL8367_VERSION				"v2.1"

#define RTL8367_DEVNAME				"rtl8367"
#define RTL8367_DEVMAJOR			(206)

#if defined(CONFIG_RTL8367_CIF_MDIO)
 #define MDIO_RTL8367_PHYID			0
#else
 #define SMI_RTL8367_SMI_ADDR			0xB8
 #define SMI_RTL8367_DELAY_NS			1700
 #define SMI_RALINK_GPIO_SDA			CONFIG_RTL8367_SMI_BUS_CPU_GPIO_DATA	/* GPIO used for SMI Data signal */
 #define SMI_RALINK_GPIO_SCK			CONFIG_RTL8367_SMI_BUS_CPU_GPIO_CLCK	/* GPIO used for SMI Clock signal */
#endif

#define WAN_PORT_X				CONFIG_RTL8367_PORT_WAN			/* 8P8C WAN  */
#define LAN_PORT_1				CONFIG_RTL8367_PORT_LAN1		/* 8P8C LAN1 */
#define LAN_PORT_2				CONFIG_RTL8367_PORT_LAN2		/* 8P8C LAN2 */
#define LAN_PORT_3				CONFIG_RTL8367_PORT_LAN3		/* 8P8C LAN3 */
#define LAN_PORT_4				CONFIG_RTL8367_PORT_LAN4		/* 8P8C LAN4 */

#if !defined(CONFIG_RAETH_GMAC2) || defined(CONFIG_RTL8367_ASIC_RVB) || defined(CONFIG_RTL8367_ASIC_R)
 #define RTL8367_SINGLE_EXTIF 1
#endif

#if defined(CONFIG_RTL8367_LAN_CPU_EXT2)
 #if defined(CONFIG_RTL8367_ASIC_RB)
  #define LAN_PORT_CPU				(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
  #define SEC_PORT_MAC				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
  #define LAN_EXT_ID				(EXT_PORT_2)
  #define WAN_EXT_ID				(EXT_PORT_1)
 #endif
#elif defined(CONFIG_RTL8367_LAN_CPU_EXT0)
 #if defined(CONFIG_RTL8367_ASIC_MB)
  #define LAN_PORT_CPU				(RTK_EXT_0_MAC)		/* ExtIf0 -> GMAC0 (5) */
  #define SEC_PORT_MAC				(RTK_EXT_1_MAC)		/* ExtIf1 -> GMAC1 (6) */
  #define LAN_EXT_ID				(EXT_PORT_0)
  #define WAN_EXT_ID				(EXT_PORT_1)
 #elif defined(CONFIG_RTL8367_ASIC_MVB)
  #define LAN_PORT_CPU				(RTK_EXT_0_MAC)		/* ExtIf0 -> GMAC0 (5) */
  #define SEC_PORT_MAC				(RTK_EXT_1_MAC)		/* ExtIf1 -> GMAC1 (6) */
  #define LAN_EXT_ID				(EXT_PORT_1)
  #define WAN_EXT_ID				(EXT_PORT_0)
 #elif defined(CONFIG_RTL8367_ASIC_M)
  #define LAN_PORT_CPU				(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
  #define SEC_PORT_MAC				(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
 #endif
#else
 #if defined(CONFIG_RTL8367_ASIC_RB)
  #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
  #define SEC_PORT_MAC				(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
  #define LAN_EXT_ID				(EXT_PORT_1)
  #define WAN_EXT_ID				(EXT_PORT_2)
 #elif defined(CONFIG_RTL8367_ASIC_RVB)
  #define LAN_PORT_CPU				(RTK_EXT_0_MAC)		/* ExtIf1 -> GMAC0 (5) */
  #define LAN_EXT_ID				(EXT_PORT_1)
 #elif defined(CONFIG_RTL8367_ASIC_R)
  #define LAN_PORT_CPU				(RTK_EXT_0_MAC9)	/* ExtIf1 -> GMAC2 (9) */
 #elif defined(CONFIG_RTL8367_ASIC_MB)
  #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> GMAC1 (6) */
  #define SEC_PORT_MAC				(RTK_EXT_0_MAC)		/* ExtIf0 -> GMAC0 (5) */
  #define LAN_EXT_ID				(EXT_PORT_1)
  #define WAN_EXT_ID				(EXT_PORT_0)
 #elif defined(CONFIG_RTL8367_ASIC_MVB)
  #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> GMAC1 (6) */
  #define SEC_PORT_MAC				(RTK_EXT_0_MAC)		/* ExtIf0 -> GMAC0 (5) */
  #define LAN_EXT_ID				(EXT_PORT_0)
  #define WAN_EXT_ID				(EXT_PORT_1)
 #elif defined(CONFIG_RTL8367_ASIC_M)
  #define LAN_PORT_CPU				(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
  #define SEC_PORT_MAC				(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
 #endif
#endif

#if defined(RTL8367_SINGLE_EXTIF)
 #define WAN_PORT_CPU				LAN_PORT_CPU
 #if defined(CONFIG_RTL8367_USE_INIC_EXT) && defined(SEC_PORT_MAC)
  #define EXT_PORT_INIC				SEC_PORT_MAC
  #define INIC_HEART_VLAN_VID			4095	/* use reserved VID for iNIC boot/heartbeat packets  */
  #define INIC_GUEST_VLAN_VID			3
  #define INIC_GUEST_FID			INIC_GUEST_VLAN_VID
  #define MIN_EXT_VLAN_VID			4
 #else
  #define MIN_EXT_VLAN_VID			3
 #endif
#else
 #define WAN_PORT_CPU				SEC_PORT_MAC
 #define MIN_EXT_VLAN_VID			3
#endif

#define RTL8367_DEFAULT_JUMBO_FRAMES		1
#define RTL8367_DEFAULT_GREEN_ETHERNET		1
#define RTL8367_DEFAULT_STORM_RATE		1024
#define RTL8367_DEFAULT_LINK_MODE		0

u32 get_phy_ports_mask_lan(u32 include_cpu);
u32 get_phy_ports_mask_wan(u32 include_cpu);

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
void igmp_init(void);
void igmp_uninit(void);
void dump_mcast_table(void);
void change_igmp_snooping_control(u32 igmp_snooping_enabled, int force_change);
void reset_igmp_snooping_table(void);
#endif


#endif

