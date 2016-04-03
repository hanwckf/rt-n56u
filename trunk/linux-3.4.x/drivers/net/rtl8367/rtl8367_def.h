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

#define RTL8367_VERSION				"v3.1"

#define RTL8367_DEVNAME				"rtl8367"

#if defined(CONFIG_RTL8367_CIF_MDIO)
 #define MDIO_RTL8367_PHYID			0
#else
 #define SMI_RALINK_GPIO_SDA			CONFIG_RTL8367_SMI_BUS_CPU_GPIO_DATA	/* GPIO used for SMI Data signal */
 #define SMI_RALINK_GPIO_SCK			CONFIG_RTL8367_SMI_BUS_CPU_GPIO_CLCK	/* GPIO used for SMI Clock signal */
#endif

#if defined(CONFIG_RAETH_BOTH_GMAC) || (defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE))
 #define RAETH_USE_BOTH_RGMII
#endif

#if defined(CONFIG_RTL8365_ASIC_MB) || defined(CONFIG_RTL8367_ASIC_RVB) || defined(CONFIG_RTL8367_ASIC_R) || \
   !defined(RAETH_USE_BOTH_RGMII) || defined(CONFIG_RTL8367_MCM_WAN_PORT)
 #define RTL8367_SINGLE_EXTIF
#endif

#if defined(CONFIG_RTL8367_LAN_CPU_EXT2)
#if defined(CONFIG_RTL8367_ASIC_RB) || defined(CONFIG_RTL8368_ASIC_MB)
 #define LAN_PORT_CPU				(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
 #define SEC_PORT_MAC				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define LAN_EXT_ID				(EXT_PORT_2)
 #define WAN_EXT_ID				(EXT_PORT_1)
#endif
#elif defined(CONFIG_RTL8367_LAN_CPU_EXT0)
#if defined(CONFIG_RTL8367_ASIC_MB) || defined(CONFIG_RTL8367_ASIC_MVB) || defined(CONFIG_RTL8368_ASIC_MB)
 #define LAN_PORT_CPU				(RTK_EXT_0_MAC)		/* ExtIf0 -> RG0 (5) */
 #define SEC_PORT_MAC				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define LAN_EXT_ID				(EXT_PORT_0)
 #define WAN_EXT_ID				(EXT_PORT_1)
#elif defined(CONFIG_RTL8367_ASIC_M) || defined(CONFIG_RTL8370_ASIC_M)
 #define LAN_PORT_CPU				(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
 #define SEC_PORT_MAC				(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
#endif
#else
#if defined(CONFIG_RTL8367_ASIC_RB)
 #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define SEC_PORT_MAC				(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
 #define LAN_EXT_ID				(EXT_PORT_1)
 #define WAN_EXT_ID				(EXT_PORT_2)
#elif defined(CONFIG_RTL8367_ASIC_MB) || defined(CONFIG_RTL8367_ASIC_MVB) || defined(CONFIG_RTL8368_ASIC_MB)
 #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define SEC_PORT_MAC				(RTK_EXT_0_MAC)		/* ExtIf0 -> RG0 (5) */
 #define LAN_EXT_ID				(EXT_PORT_1)
 #define WAN_EXT_ID				(EXT_PORT_0)
#elif defined(CONFIG_RTL8367_ASIC_M) || defined(CONFIG_RTL8370_ASIC_M)
 #define LAN_PORT_CPU				(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
 #define SEC_PORT_MAC				(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
#endif
#endif

/* single Ext port */
#if defined(CONFIG_RTL8365_ASIC_MB)
 #define LAN_PORT_CPU				(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define LAN_EXT_ID				(EXT_PORT_1)
#elif defined(CONFIG_RTL8367_ASIC_RVB)
 #define LAN_PORT_CPU				(RTK_EXT_0_MAC)		/* ExtIf1 -> RG0 (5) */
 #define LAN_EXT_ID				(EXT_PORT_1)
#elif defined(CONFIG_RTL8367_ASIC_R)
 #define LAN_PORT_CPU				(RTK_EXT_0_MAC9)	/* ExtIf1 -> GMAC2 (9) */
#endif

#if defined(RTL8367_SINGLE_EXTIF)
 #define WAN_PORT_CPU				LAN_PORT_CPU
 #if defined(CONFIG_RTL8367_USE_INIC_EXT) && defined(SEC_PORT_MAC)
  #define EXT_PORT_INIC				SEC_PORT_MAC
  #define INIC_HEART_VLAN_VID			4095	/* use reserved VID for iNIC boot/heartbeat packets  */
  #define INIC_GUEST_VLAN_VID			3
  #define INIC_GUEST_VLAN_FID			INIC_GUEST_VLAN_VID
  #define MIN_EXT_VLAN_VID			4
 #else
  #define MIN_EXT_VLAN_VID			2
 #endif
#else
 #define WAN_PORT_CPU				SEC_PORT_MAC
 #define MIN_EXT_VLAN_VID			2
#endif

#define WAN_PORT_X				CONFIG_RTL8367_PORT_WAN			/* 8P8C WAN  */
#define LAN_PORT_1				CONFIG_RTL8367_PORT_LAN1		/* 8P8C LAN1 */
#define LAN_PORT_2				CONFIG_RTL8367_PORT_LAN2		/* 8P8C LAN2 */
#define LAN_PORT_3				CONFIG_RTL8367_PORT_LAN3		/* 8P8C LAN3 */
#define LAN_PORT_4				CONFIG_RTL8367_PORT_LAN4		/* 8P8C LAN4 */

#define MASK_WAN_PORT_CPU			(1u << WAN_PORT_CPU)
#define MASK_LAN_PORT_CPU			(1u << LAN_PORT_CPU)
#define MASK_LAN_PORT_1				(1u << LAN_PORT_1)
#define MASK_LAN_PORT_2				(1u << LAN_PORT_2)
#define MASK_LAN_PORT_3				(1u << LAN_PORT_3)
#define MASK_LAN_PORT_4				(1u << LAN_PORT_4)

#if defined(CONFIG_RTL8367_MCM_WAN_PORT)
 #define MCM_WAN_PORT_X				CONFIG_RTL8367_MCM_WAN_PORT_ID
 #define MASK_WAN_PORT_X			0
#else
 #define MASK_WAN_PORT_X			(1u << WAN_PORT_X)
#endif

#if defined(CONFIG_RTL8370_ASIC_M)
 #define LAN_PORT_5				CONFIG_RTL8367_PORT_LAN5
 #define LAN_PORT_6				CONFIG_RTL8367_PORT_LAN6
 #define LAN_PORT_7				CONFIG_RTL8367_PORT_LAN7
 #define MASK_LAN_PORT_5			(1u << LAN_PORT_5)
 #define MASK_LAN_PORT_6			(1u << LAN_PORT_6)
 #define MASK_LAN_PORT_7			(1u << LAN_PORT_7)
#else
 #define MASK_LAN_PORT_5			0
 #define MASK_LAN_PORT_6			0
 #define MASK_LAN_PORT_7			0
#endif

#define MASK_LAN_PORTS_ALL			(MASK_LAN_PORT_1|MASK_LAN_PORT_2|MASK_LAN_PORT_3|MASK_LAN_PORT_4| \
						 MASK_LAN_PORT_5|MASK_LAN_PORT_6|MASK_LAN_PORT_7)

#if defined(CONFIG_RTL8367_ASIC_RB)
 #define ASIC_NAME "RTL8367RB"
#elif defined(CONFIG_RTL8367_ASIC_RVB)
 #define ASIC_NAME "RTL8367R-VB"
#elif defined(CONFIG_RTL8367_ASIC_MVB)
 #define ASIC_NAME "RTL8367M-VB"
#elif defined(CONFIG_RTL8367_ASIC_MB)
 #define ASIC_NAME "RTL8367MB"
#elif defined(CONFIG_RTL8365_ASIC_MB)
 #define ASIC_NAME "RTL8365MB"
#elif defined(CONFIG_RTL8368_ASIC_MB)
 #define ASIC_NAME "RTL8368MB"
#elif defined(CONFIG_RTL8367_ASIC_R)
 #define ASIC_NAME "RTL8367R"
#elif defined(CONFIG_RTL8367_ASIC_M)
 #define ASIC_NAME "RTL8367M"
#elif defined(CONFIG_RTL8370_ASIC_M)
 #define ASIC_NAME "RTL8370M"
#else
 #define ASIC_NAME "RTL8367"
#endif

////////////////////////////////////////////////////////////////////////////////////

#define RTL8367_DEFAULT_JUMBO_FRAMES		1
#define RTL8367_DEFAULT_GREEN_ETHERNET		1
#define RTL8367_DEFAULT_EEE_LPI			0
#define RTL8367_DEFAULT_STORM_RATE		1024
#define RTL8367_DEFAULT_LINK_MODE		0

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
	u32 fid:12;
	u32 unused:7;
	u32 port_member:16;
	u32 port_untag:16;
} vlan_entry_t;

////////////////////////////////////////////////////////////////////////////////////

u32 get_phy_ports_mask_lan(u32 include_cpu);
u32 get_phy_ports_mask_wan(u32 include_cpu);
u32 get_ports_mask_from_uapi(u32 user_port_mask);

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
void igmp_sn_init(void);
void igmp_sn_uninit(void);
void igmp_sn_set_enable(u32 igmp_sn_enabled);
void igmp_sn_set_static_ports(u32 ports_mask);
void igmp_sn_dump_mcast_table(void);
#endif


#endif

