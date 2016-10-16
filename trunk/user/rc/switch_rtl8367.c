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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include "rc.h"
#include "switch.h"

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

typedef struct mib_counters_s
{
    uint64_t ifInOctets;
    uint32_t dot3StatsFCSErrors;
    uint32_t dot3StatsSymbolErrors;
    uint32_t dot3InPauseFrames;
    uint32_t dot3ControlInUnknownOpcodes;
    uint32_t etherStatsFragments;
    uint32_t etherStatsJabbers;
    uint32_t ifInUcastPkts;
    uint32_t etherStatsDropEvents;
    uint64_t etherStatsOctets;
    uint32_t etherStatsUndersizePkts;
    uint32_t etherStatsOversizePkts;
    uint32_t etherStatsPkts64Octets;
    uint32_t etherStatsPkts65to127Octets;
    uint32_t etherStatsPkts128to255Octets;
    uint32_t etherStatsPkts256to511Octets;
    uint32_t etherStatsPkts512to1023Octets;
    uint32_t etherStatsPkts1024toMaxOctets;
    uint32_t etherStatsMcastPkts;
    uint32_t etherStatsBcastPkts;
    uint64_t ifOutOctets;
    uint32_t dot3StatsSingleCollisionFrames;
    uint32_t dot3StatsMultipleCollisionFrames;
    uint32_t dot3StatsDeferredTransmissions;
    uint32_t dot3StatsLateCollisions;
    uint32_t etherStatsCollisions;
    uint32_t dot3StatsExcessiveCollisions;
    uint32_t dot3OutPauseFrames;
    uint32_t dot1dBasePortDelayExceededDiscards;
    uint32_t dot1dTpPortInDiscards;
    uint32_t ifOutUcastPkts;
    uint32_t ifOutMulticastPkts;
    uint32_t ifOutBrocastPkts;
    uint32_t outOampduPkts;
    uint32_t inOampduPkts;
    uint32_t pktgenPkts;
} mib_counters_t;

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

static int
rtl8367_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(RTL8367_DEVPATH, O_RDONLY);
	if (fd < 0) {
		perror(RTL8367_DEVPATH);
		return errno;
	}

	cmd &= ((1u << RTL8367_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << RTL8367_IOCTL_CMD_LENGTH_BITS);

	if (ioctl(fd, cmd, value) < 0) {
		perror("ioctl");
		retVal = errno;
	}

	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// RTL8367 STATUS
////////////////////////////////////////////////////////////////////////////////

int phy_status_port_link(int port_id_uapi, unsigned int *p_link_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STATUS_LINK_PORT, port_id_uapi, p_link_on);
}

int phy_status_port_link_wan_all(unsigned int *p_link_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STATUS_LINK_PORTS_WAN, 0, p_link_on);
}

int phy_status_port_link_lan_all(unsigned int *p_link_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STATUS_LINK_PORTS_LAN, 0, p_link_on);
}

int phy_status_port_link_changed(unsigned int *p_link_changed)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STATUS_LINK_CHANGED, 0, p_link_changed);
}

////////////////////////////////////////////////////////////////////////////////

int phy_status_port_speed(int port_id_uapi, unsigned int *p_speed_mode)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STATUS_SPEED_PORT, port_id_uapi, p_speed_mode);
}

////////////////////////////////////////////////////////////////////////////////

int phy_set_link_port(int port_id_uapi, unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return rtl8367_ioctl(RTL8367_IOCTL_SPEED_PORT, port_id_uapi, &port_control);
}

////////////////////////////////////////////////////////////////////////////////

int phy_status_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx)
{
	port_bytes_t pb;
	int ioctl_result;

	ioctl_result = rtl8367_ioctl(RTL8367_IOCTL_STATUS_BYTES_PORT, port_id_uapi, (unsigned int *)&pb);
	if (ioctl_result < 0)
		return ioctl_result;

	*rx = pb.RX;
	*tx = pb.TX;

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////

int phy_storm_unicast_unknown(unsigned int storm_rate_mbps)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STORM_UNICAST_UNK, 0, &storm_rate_mbps);
}

int phy_storm_multicast_unknown(unsigned int storm_rate_mbps)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STORM_MULTICAST_UNK, 0, &storm_rate_mbps);
}

int phy_storm_multicast(unsigned int storm_rate_mbps)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STORM_MULTICAST, 0, &storm_rate_mbps);
}

int phy_storm_broadcast(unsigned int storm_rate_mbps)
{
	return rtl8367_ioctl(RTL8367_IOCTL_STORM_BROADCAST, 0, &storm_rate_mbps);
}

////////////////////////////////////////////////////////////////////////////////

int phy_led_mode_green(unsigned int led_green)
{
	return rtl8367_ioctl(RTL8367_IOCTL_LED_MODE_GROUP0, 0, &led_green);
}

int phy_led_mode_yellow(unsigned int led_yellow)
{
	return rtl8367_ioctl(RTL8367_IOCTL_LED_MODE_GROUP1, 0, &led_yellow);
}

////////////////////////////////////////////////////////////////////////////////

int phy_green_ethernet(unsigned int green_ethernet_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_GREEN_ETHERNET, 0, &green_ethernet_on);
}

int phy_eee_lpi(unsigned int eee_lpi_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_EEE_LPI, 0, &eee_lpi_on);
}

int phy_jumbo_frames(unsigned int jumbo_frames_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_JUMBO_FRAMES, 0, &jumbo_frames_on);
}

int phy_igmp_static_port(int static_port)
{
	unsigned int ports_mask = 0;

	if (static_port >= 0 && static_port <= 13)
		ports_mask |= (1u << static_port);

	return rtl8367_ioctl(RTL8367_IOCTL_IGMP_STATIC_PORTS, 0, &ports_mask);
}

int phy_igmp_snooping(unsigned int igmp_snooping_on)
{
	return rtl8367_ioctl(RTL8367_IOCTL_IGMP_SNOOPING, 0, &igmp_snooping_on);
}

////////////////////////////////////////////////////////////////////////////////

int phy_ports_power(int power_on)
{
	unsigned int ports_mask = (SWAPI_PORTMASK_LAN1 |
				   SWAPI_PORTMASK_LAN2 |
				   SWAPI_PORTMASK_LAN3 |
				   SWAPI_PORTMASK_LAN4 |
				   SWAPI_PORTMASK_LAN5 |
				   SWAPI_PORTMASK_LAN6 |
				   SWAPI_PORTMASK_LAN7 |
				   SWAPI_PORTMASK_WAN);
	return rtl8367_ioctl(RTL8367_IOCTL_PORTS_POWER, power_on, &ports_mask);
}

int phy_ports_wan_power(int power_on)
{
	unsigned int ports_wan = 1;
	return rtl8367_ioctl(RTL8367_IOCTL_PORTS_WAN_LAN_POWER, power_on, &ports_wan);
}

int phy_ports_lan_power(int power_on)
{
	unsigned int ports_wan = 0;
	return rtl8367_ioctl(RTL8367_IOCTL_PORTS_WAN_LAN_POWER, power_on, &ports_wan);
}

////////////////////////////////////////////////////////////////////////////////

int phy_bridge_mode(unsigned int bridge_mode, int isolated_mode)
{
	return rtl8367_ioctl(RTL8367_IOCTL_BRIDGE_MODE, isolated_mode, &bridge_mode);
}

int phy_isolate_inic(unsigned int inic_isolated)
{
	return rtl8367_ioctl(RTL8367_IOCTL_ISOLATE_INIC, 0, &inic_isolated);
}

int phy_disable_inic(unsigned int inic_disabled)
{
	return rtl8367_ioctl(RTL8367_IOCTL_DISABLE_INIC, 0, &inic_disabled);
}

inline int phy_clear_mac_table(void)
{
	unsigned int unused = 0;
	return rtl8367_ioctl(RTL8367_IOCTL_MAC_TABLE_CLEAR, 0, &unused);
}

int phy_vlan_reset_table(void)
{
	unsigned int unused = 0;
	return rtl8367_ioctl(RTL8367_IOCTL_VLAN_RESET_TABLE, 0, &unused);
}

int phy_vlan_pvid_wan_get(void)
{
	unsigned int pvid = 2;
	rtl8367_ioctl(RTL8367_IOCTL_VLAN_PVID_WAN_GET, 0, &pvid);
	return (int)pvid;
}

int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask)
{
	return rtl8367_ioctl(RTL8367_IOCTL_VLAN_ACCEPT_PORT_MODE, accept_mode, &port_pask);
}

int phy_vlan_create_port_vid(int pvid, int priority, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0x0FFF) << 16) | ((priority & 0x07) << 12) | (pvid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFFFF) << 16) | (member & 0xFFFF));
	
	return rtl8367_ioctl(RTL8367_IOCTL_VLAN_CREATE_PORT_VID, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_create_entry(int vid, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0x0FFF) << 16) | (vid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFFFF) << 16) | (member & 0xFFFF));
	
	return rtl8367_ioctl(RTL8367_IOCTL_VLAN_CREATE_ENTRY, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_rule_set(unsigned int rule_id, int vid, int priority, int tagged)
{
	unsigned int vlan_rule = (((tagged & 0xFF) << 24) | ((priority & 0xFF) << 16) | (vid & 0xFFFF));
	
	return rtl8367_ioctl(RTL8367_IOCTL_VLAN_RULE_SET, rule_id, &vlan_rule);
}

////////////////////////////////////////////////////////////////////////////////
// STATUS
////////////////////////////////////////////////////////////////////////////////

static int show_usage(char *cmd)
{
	printf("Usage: %s COMMAND [ARG1] [ARG2]\n"
	" COMMAND:\n"
	"   10 [PORT]        Show port link status\n"
	"   11               Show WAN ports link status\n"
	"   12               Show LAN ports link status\n"
	"\n"
	"   20 [PORT]        Show port speed status\n"
	"\n"
	"   30 [PORT]        Show port MIB counters\n"
	"   38               Reset all ports MIB counters\n"
	"\n"
	"   40 [0x25252525]  Full reset and reinit switch\n"
	"   41 [MASK] [0|1]  Set power off/on for ports mask\n"
	"   42 [W|L]  [0|1]  Set power off/on for WAN or LAN ports\n"
#if defined(USE_RTL8367_API_8367B)
	"   43               Clear switch L2 MAC table\n"
#endif
	"\n"
	"   50 [0..8] [0..3] Config WAN bridge mode and isolation\n"
#if defined (USE_RT3352_MII)
	"   51 [0|1]         Toggle iNIC isolation from LAN ports\n"
	"   52 [0|1]         Toggle iNIC disable RGMII port link\n"
#endif
	"   55 [MASK] [PORT] Override port forward mask\n"
	"\n"
	"   60               Reset VLAN table and init VLAN1\n"
	"   61               Show untagged WAN PVID\n"
	"   62 [MASK] [0..2] Set VLAN accept mode for ports mask\n"
	"   63 [MASK] [DATA] Create port-based VLAN entry\n"
	"   64 [MASK] [DATA] Create VLAN entry\n"
	"\n"
	"   70 [1..1024]     Set Unknown Unicast storm rate limit for all ports\n"
	"   71 [1..1024]     Set Unknown Multicast storm rate limit for all ports\n"
	"   72 [1..1024]     Set Multicast storm rate limit for all ports\n"
	"   73 [1..1024]     Set Broadcast storm rate limit for all ports\n"
	"\n"
	"   75 [1|0]         Set Jumbo Frames accept on/off\n"
	"   76 [1|0]         Set Green Ethernet on/off\n"
	"   77 [1|0]         Set 802.3az EEE on/off\n"
	"\n"
#if defined(USE_RTL8367_IGMP_SNOOPING)
	"   78 [1|0]         Set IGMP/MLD snooping on/off\n"
	"   79 [MASK]        Set IGMP/MLD static ports mask\n"
#endif
	"\n"
	"   80 [0..11]       Set LED action group0\n"
	"   81 [0..11]       Set LED action group1\n"
	"   82 [0..11]       Set LED action group2\n"
	"\n"
	"   90 [MODE] [PORT] Set port link mode (fc|link)\n"
	"\n"
	"  100 [0..7]        Set ExtIf RGMII delay RX\n"
	"  101 [0..1]        Set ExtIf RGMII delay TX\n"
	"\n"
#if defined(USE_RTL8367_IGMP_SNOOPING)
	"  110               Dump IGMP/MLD static LUT entries from ASIC\n"
#endif
#if defined(USE_RTL8367_API_8367B)
	"  111               Dump L2 MAC entries from ASIC\n"
#endif
	"  112               Dump ASIC ports isolation\n"
	"\n"
	"  120 [REG]         Get ASIC register\n"
	"  121 [REG] [VALUE] Set ASIC register\n"
	"  122 [REG]         Get PHY register\n"
	"  123 [REG] [VALUE] Set PHY register\n"
	, cmd);

	return 1;
}

static const char *get_port_name(unsigned int port_id)
{
	switch (port_id)
	{
	case SWAPI_PORT_WAN:
		return "WAN";
	case SWAPI_PORT_LAN1:
		return "LAN1";
	case SWAPI_PORT_LAN2:
		return "LAN2";
	case SWAPI_PORT_LAN3:
		return "LAN3";
	case SWAPI_PORT_LAN4:
		return "LAN4";
	case SWAPI_PORT_LAN5:
		return "LAN5";
	case SWAPI_PORT_LAN6:
		return "LAN6";
	case SWAPI_PORT_LAN7:
		return "LAN7";
	case SWAPI_PORT_CPU_INIC:
		return "CPU iNIC";
	case SWAPI_PORT_CPU_LAN:
		return "CPU LAN";
	case SWAPI_PORT_CPU_WAN:
		return "CPU WAN";
	}

	return NULL;
}

static int show_status_link(unsigned int cmd, unsigned int port_id)
{
	int retVal;
	unsigned int link_value = 0;
	const char *portname = NULL;

	retVal = rtl8367_ioctl(cmd, port_id, &link_value);
	if (retVal != 0)
		return retVal;

	switch (cmd)
	{
	case RTL8367_IOCTL_STATUS_LINK_PORT:
		portname = get_port_name(port_id);
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_WAN:
		portname = "WAN ports";
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_LAN:
		portname = "LAN ports";
		break;
	}

	if (!portname)
		return -1;

	printf("%s link state: %d\n", portname, link_value);

	return retVal;
}

static int show_status_speed(unsigned int port_id)
{
	int retVal;
	unsigned int link_value = 0;
	const char *portname;
	char linkstate[32];

	retVal = rtl8367_ioctl(RTL8367_IOCTL_STATUS_SPEED_PORT, port_id, &link_value);
	if (retVal != 0)
		return retVal;

	portname = get_port_name(port_id);
	if (!portname)
		return -1;

	if ((link_value >> 16) & 0x01) {
		int lspeed;
		const char *text_fc = "";
		const char *text_dup = "HD";
		const char *text_eee = "";
		
		switch (link_value & 0x03)
		{
		case 2:
			lspeed = 1000;
			break;
		case 1:
			lspeed = 100;
			break;
		default:
			lspeed = 10;
			break;
		}
		
		if ((link_value >> 8) & 0x01) {
			unsigned int link_fc = (link_value >> 9) & 0x03;
			if (link_fc)
				text_fc = ", FC";
			text_dup = "FD";
		}
		
		if ((link_value >> 11) & 0x03)
			text_eee = ", EEE";
		
		/* 1000FD, FC */
		snprintf(linkstate, sizeof(linkstate), "%d%s%s%s", lspeed, text_dup, text_fc, text_eee);
	} else
		strcpy(linkstate, "NO");

	printf("%s link: %s\n", portname, linkstate);

	return retVal;
}

static int show_mib_counters(unsigned int port_id)
{
	int retVal;
	mib_counters_t mibc;
	const char *portname;

	memset(&mibc, 0, sizeof(mib_counters_t));
	retVal = rtl8367_ioctl(RTL8367_IOCTL_STATUS_MIB_PORT, port_id, (unsigned int *)&mibc);
	if (retVal != 0)
		return retVal;

	portname = get_port_name(port_id);
	if (!portname)
		return -1;

		printf("%s MIB counters:\n"
			"  ifInOctets: %llu\n"
			"  ifInUcastPkts: %u\n"
			"  ifOutOctets: %llu\n"
			"  ifOutUcastPkts: %u\n"
			"  ifOutMulticastPkts: %u\n"
			"  ifOutBrocastPkts: %u\n"
			"  etherStatsOctets: %llu\n"
			"  etherStatsMcastPkts: %u\n"
			"  etherStatsBcastPkts: %u\n"
			"  etherStatsFragments: %u\n"
			"  etherStatsJabbers: %u\n"
			"  etherStatsDropEvents: %u\n"
			"  etherStatsUndersizePkts: %u\n"
			"  etherStatsOversizePkts: %u\n"
			"  etherStatsCollisions: %u\n"
			"  dot3StatsFCSErrors: %u\n"
			"  dot3StatsSymbolErrors: %u\n"
			"  dot3ControlInUnknownOpcodes: %u\n"
			"  dot3StatsSingleCollisionFrames: %u\n"
			"  dot3StatsMultipleCollisionFrames: %u\n"
			"  dot3StatsLateCollisions: %u\n"
			"  dot3StatsExcessiveCollisions: %u\n"
			"  dot3StatsDeferredTransmissions: %u\n"
			"  dot3InPauseFrames: %u\n"
			"  dot3OutPauseFrames: %u\n"
			"  dot1dTpPortInDiscards: %u\n"
			"  dot1dBasePortDelayExceededDiscards: %u\n"
			,
			portname,
			mibc.ifInOctets,
			mibc.ifInUcastPkts,
			mibc.ifOutOctets,
			mibc.ifOutUcastPkts,
			mibc.ifOutMulticastPkts,
			mibc.ifOutBrocastPkts,
			mibc.etherStatsOctets,
			mibc.etherStatsMcastPkts,
			mibc.etherStatsBcastPkts,
			mibc.etherStatsFragments,
			mibc.etherStatsJabbers,
			mibc.etherStatsDropEvents,
			mibc.etherStatsUndersizePkts,
			mibc.etherStatsOversizePkts,
			mibc.etherStatsCollisions,
			mibc.dot3StatsFCSErrors,
			mibc.dot3StatsSymbolErrors,
			mibc.dot3ControlInUnknownOpcodes,
			mibc.dot3StatsSingleCollisionFrames,
			mibc.dot3StatsMultipleCollisionFrames,
			mibc.dot3StatsLateCollisions,
			mibc.dot3StatsExcessiveCollisions,
			mibc.dot3StatsDeferredTransmissions,
			mibc.dot3InPauseFrames,
			mibc.dot3OutPauseFrames,
			mibc.dot1dTpPortInDiscards,
			mibc.dot1dBasePortDelayExceededDiscards
			);

	return retVal;
}

static int show_vlan_pvid_wan(void)
{
	int retVal;
	unsigned int arg = 2;

	retVal = rtl8367_ioctl(RTL8367_IOCTL_VLAN_PVID_WAN_GET, 0, &arg);
	if (retVal != 0)
		arg = 2;

	printf("%d\n", arg);

	return retVal;
}

static int asic_reg_get(unsigned int cmd, unsigned int reg)
{
	int retVal;
	unsigned int reg_value = 0;

	retVal = rtl8367_ioctl(cmd, reg, &reg_value);
	if (retVal != 0)
		return retVal;

	switch (cmd)
	{
	case RTL8367_IOCTL_REG_GET:
		printf("Get ASIC reg 0x%04X, value: 0x%04X\n", reg, reg_value);
		break;
	case RTL8367_IOCTL_PHY_REG_GET:
		printf("Get PHY %d, reg 0x%02X, value: 0x%04X\n", (reg >> 8) & 0x1f, reg & 0x1f, reg_value);
		break;
	}

	return retVal;
}

static int asic_reg_set(unsigned int cmd, unsigned int reg, unsigned int reg_value)
{
	int retVal;

	retVal = rtl8367_ioctl(cmd, reg, &reg_value);
	if (retVal != 0)
		return retVal;

	switch (cmd)
	{
	case RTL8367_IOCTL_REG_SET:
		printf("Set ASIC reg 0x%04X, value: 0x%04X\n", reg, reg_value);
		break;
	case RTL8367_IOCTL_PHY_REG_SET:
		printf("Set PHY %d, reg 0x%02X, value: 0x%04X\n", (reg >> 8) & 0x1f, reg & 0x1f, reg_value);
		break;
	}

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// RTL8367 PROCESS
////////////////////////////////////////////////////////////////////////////////

int rtl8367_node(void)
{
	return mknod(RTL8367_DEVPATH, S_IFCHR | 0666, makedev(RTL8367_DEVMAJOR, 0));
}

int rtl8367_main(int argc, char **argv)
{
	unsigned int cmd, par = 0, arg = 0;

	if (argc < 2)
		return show_usage(argv[0]);

	if (!isdigit(argv[1][0]))
		return show_usage(argv[0]);

	cmd = atoi(argv[1]);

	if (argc > 2)
		arg = get_param_int_hex(argv[2]);
	if (argc > 3)
		par = get_param_int_hex(argv[3]);

	switch (cmd)
	{
	case RTL8367_IOCTL_STATUS_BYTES_PORT:
		return 1;
	
	case RTL8367_IOCTL_STATUS_LINK_PORT:
		return show_status_link(cmd, arg);
	
	case RTL8367_IOCTL_STATUS_SPEED_PORT:
		return show_status_speed(arg);
	
	case RTL8367_IOCTL_STATUS_MIB_PORT:
		return show_mib_counters(arg);
	
	case RTL8367_IOCTL_VLAN_PVID_WAN_GET:
		return show_vlan_pvid_wan();

	case RTL8367_IOCTL_REG_GET:
	case RTL8367_IOCTL_PHY_REG_GET:
		return asic_reg_get(cmd, arg);

	case RTL8367_IOCTL_REG_SET:
	case RTL8367_IOCTL_PHY_REG_SET:
		return asic_reg_set(cmd, arg, par);
	}

	return rtl8367_ioctl(cmd, par, &arg);
}

