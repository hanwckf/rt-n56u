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

#if defined (USE_MTK_GSW)
typedef struct esw_mib_counters_s
{
	uint64_t TxGoodOctets;
	uint32_t TxUcastFrames;
	uint32_t TxMcastFrames;
	uint32_t TxBcastFrames;
	uint32_t TxDropFrames;
	uint32_t TxPauseFrames;
	uint32_t TxCollisions;
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
#elif defined (CONFIG_RALINK_MT7620)
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
#else
#define ESW_RT3X5X
typedef struct esw_mib_counters_s
{
	uint32_t TxGoodFrames;
	uint32_t TxBadFrames;
	uint32_t RxGoodFrames;
	uint32_t RxBadFrames;
} esw_mib_counters_t;
#endif

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

static int
mtk_esw_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(MTK_ESW_DEVPATH, O_RDONLY);
	if (fd < 0)
	{
		perror(MTK_ESW_DEVPATH);
		return errno;
	}

	cmd &= ((1u << MTK_ESW_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << MTK_ESW_IOCTL_CMD_LENGTH_BITS);

	if (ioctl(fd, cmd, value) < 0) {
		perror("ioctl");
		retVal = errno;
	}

	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// ESW STATUS
////////////////////////////////////////////////////////////////////////////////

int phy_status_port_link(int port_id_uapi, unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT, port_id_uapi, p_link_on);
}

int phy_status_port_link_wan_all(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN, 0, p_link_on);
}

int phy_status_port_link_lan_all(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN, 0, p_link_on);
}

int phy_status_port_link_changed(unsigned int *p_link_changed)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_CHANGED, 0, p_link_changed);
}

////////////////////////////////////////////////////////////////////////////////

int phy_status_port_speed(int port_id_uapi, unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT, port_id_uapi, p_speed_mode);
}

////////////////////////////////////////////////////////////////////////////////

int phy_set_link_port(int port_id_uapi, unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT, port_id_uapi, &port_control);
}

////////////////////////////////////////////////////////////////////////////////

int phy_status_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx)
{
	port_bytes_t pb;
	int ioctl_result;

	ioctl_result = mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_BYTES_PORT, port_id_uapi, (unsigned int *)&pb);
	if (ioctl_result < 0)
		return ioctl_result;

	*rx = pb.RX;
	*tx = pb.TX;

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////

inline int phy_storm_unicast_unknown(unsigned int storm_rate_mbps)
{
	// N.A.
	return 0;
}

inline int phy_storm_multicast_unknown(unsigned int storm_rate_mbps)
{
	// N.A.
	return 0;
}

inline int phy_storm_multicast(unsigned int storm_rate_mbps)
{
	// N.A.
	return 0;
}

int phy_storm_broadcast(unsigned int storm_rate_mbps)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STORM_BROADCAST, 0, &storm_rate_mbps);
}

////////////////////////////////////////////////////////////////////////////////

int phy_led_mode_green(unsigned int led_green)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_LED_MODE, 0, &led_green);
}

inline int phy_led_mode_yellow(unsigned int led_yellow)
{
	// N.A.
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

inline int phy_green_ethernet(unsigned int green_ethernet_on)
{
	// N.A.
	return 0;
}

int phy_eee_lpi(unsigned int eee_lpi_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_EEE_LPI, 0, &eee_lpi_on);
}

int phy_jumbo_frames(unsigned int jumbo_frames_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_JUMBO_FRAMES, 0, &jumbo_frames_on);
}

int phy_igmp_static_port(int static_port)
{
	unsigned int ports_mask = 0;

	if (static_port >= 0 && static_port <= 5)
		ports_mask |= (1u << static_port);

	return mtk_esw_ioctl(MTK_ESW_IOCTL_IGMP_STATIC_PORTS, 0, &ports_mask);
}

int phy_igmp_snooping(unsigned int igmp_snooping_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_IGMP_SNOOPING, 0, &igmp_snooping_on);
}

////////////////////////////////////////////////////////////////////////////////

int phy_ports_power(int power_on)
{
	unsigned int ports_mask = (SWAPI_PORTMASK_LAN1 |
				   SWAPI_PORTMASK_LAN2 |
				   SWAPI_PORTMASK_LAN3 |
				   SWAPI_PORTMASK_LAN4 |
				   SWAPI_PORTMASK_LAN5 |
				   SWAPI_PORTMASK_WAN);
	return mtk_esw_ioctl(MTK_ESW_IOCTL_PORTS_POWER, power_on, &ports_mask);
}

int phy_ports_wan_power(int power_on)
{
	unsigned int ports_wan = 1;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_PORTS_WAN_LAN_POWER, power_on, &ports_wan);
}

int phy_ports_lan_power(int power_on)
{
	unsigned int ports_wan = 0;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_PORTS_WAN_LAN_POWER, power_on, &ports_wan);
}

////////////////////////////////////////////////////////////////////////////////

int phy_bridge_mode(unsigned int bridge_mode, int isolated_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_BRIDGE_MODE, isolated_mode, &bridge_mode);
}

inline int phy_isolate_inic(unsigned int inic_isolated)
{
	// N.A.
	return 0;
}

inline int phy_disable_inic(unsigned int inic_disabled)
{
	// N.A.
	return 0;
}

int phy_clear_mac_table(void)
{
	unsigned int unused = 0;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_MAC_TABLE_CLEAR, 0, &unused);
}

int phy_vlan_reset_table(void)
{
	unsigned int unused = 0;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_RESET_TABLE, 0, &unused);
}

int phy_vlan_pvid_wan_get(void)
{
	unsigned int pvid = 2;
	mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_PVID_WAN_GET, 0, &pvid);
	return (int)pvid;
}

int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_ACCEPT_PORT_MODE, accept_mode, &port_pask);
}

int phy_vlan_create_port_vid(int pvid, int priority, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0xFF) << 16) | ((priority & 0x07) << 12) | (pvid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFFFF) << 16) | (member & 0xFFFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_CREATE_PORT_VID, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_create_entry(int vid, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0xFF) << 16) | (vid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFFFF) << 16) | (member & 0xFFFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_CREATE_ENTRY, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_rule_set(unsigned int rule_id, int vid, int priority, int tagged)
{
	unsigned int vlan_rule = (((tagged & 0xFF) << 24) | ((priority & 0xFF) << 16) | (vid & 0xFFFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_RULE_SET, rule_id, &vlan_rule);
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
	"   43               Clear switch L2 MAC table\n"
	"\n"
	"   50 [0..8] [0..3] Config WAN bridge mode and isolation\n"
#if defined (USE_MTK_GSW)
	"   55 [MASK] [PORT] Override port matrix mask\n"
#endif
	"\n"
	"   60               Reset VLAN table and init VLAN1\n"
	"   61               Show untagged WAN PVID\n"
	"   62 [MASK] [0..2] Set VLAN accept mode for ports mask\n"
	"   63 [MASK] [DATA] Create port-based VLAN entry\n"
	"   64 [MASK] [DATA] Create VLAN entry\n"
	"\n"
	"   73 [0..1000]     Set Broadcast storm rate limit for all PHY ports\n"
#if !defined (ESW_RT3X5X)
	"   75 [1|0]         Set Jumbo Frames accept on/off\n"
#endif
	"   77 [1|0]         Set 802.3az EEE on/off\n"
	"\n"
	"   78 [1|0]         Set IGMP/MLD snooping on/off\n"
	"   79 [MASK]        Set IGMP/MLD static ports mask\n"
	"\n"
	"   80 [7,11]        Set EPHY LED action\n"
	"\n"
	"   90 [MODE] [PORT] Set port link mode (fc|link)\n"
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

	retVal = mtk_esw_ioctl(cmd, port_id, &link_value);
	if (retVal != 0)
		return retVal;

	switch (cmd)
	{
	case MTK_ESW_IOCTL_STATUS_LINK_PORT:
		portname = get_port_name(port_id);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
		portname = "WAN ports";
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
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

	retVal = mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT, port_id, &link_value);
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
		case 3:
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
		
		/* 1000FD, FC, EEE */
		snprintf(linkstate, sizeof(linkstate), "%d%s%s%s", lspeed, text_dup, text_fc, text_eee);
	} else
		strcpy(linkstate, "NO");

	printf("%s link: %s\n", portname, linkstate);

	return retVal;
}

static int show_mib_counters(unsigned int port_id)
{
	int retVal;
	esw_mib_counters_t mibc;
	const char *portname;

	memset(&mibc, 0, sizeof(esw_mib_counters_t));
	retVal = mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_MIB_PORT, port_id, (unsigned int *)&mibc);
	if (retVal != 0)
		return retVal;

	portname = get_port_name(port_id);
	if (!portname)
		return -1;

#if defined (USE_MTK_GSW)
		printf("%s MIB counters:\n"
			"  TxGoodOctets: %llu\n"
			"  TxUcastFrames: %u\n"
			"  TxMcastFrames: %u\n"
			"  TxBcastFrames: %u\n"
			"  TxDropFrames: %u\n"
			"  TxPauseFrames: %u\n"
			"  TxCollisions: %u\n"
			"  TxCRCError: %u\n"
			"  RxGoodOctets: %llu\n"
			"  RxUcastFrames: %u\n"
			"  RxMcastFrames: %u\n"
			"  RxBcastFrames: %u\n"
			"  RxDropFrames: %u\n"
			"  RxPauseFrames: %u\n"
			"  RxFilterFrames: %u\n"
			"  RxCRCError: %u\n"
			"  RxAligmentError: %u\n"
			,
			portname,
			mibc.TxGoodOctets,
			mibc.TxUcastFrames,
			mibc.TxMcastFrames,
			mibc.TxBcastFrames,
			mibc.TxDropFrames,
			mibc.TxPauseFrames,
			mibc.TxCollisions,
			mibc.TxCRCError,
			mibc.RxGoodOctets,
			mibc.RxUcastFrames,
			mibc.RxMcastFrames,
			mibc.RxBcastFrames,
			mibc.RxDropFrames,
			mibc.RxPauseFrames,
			mibc.RxFilterFrames,
			mibc.RxCRCError,
			mibc.RxAligmentError
			);
#elif !defined (ESW_RT3X5X)
		printf("%s MIB counters:\n"
			"  TxGoodOctets: %llu\n"
			"  TxGoodFrames: %u\n"
			"  TxBadOctets: %u\n"
			"  TxBadFrames: %u\n"
			"  TxDropFrames: %u\n"
			"  RxGoodOctets: %llu\n"
			"  RxGoodFrames: %u\n"
			"  RxBadOctets: %u\n"
			"  RxBadFrames: %u\n"
			"  RxDropFramesFilter: %u\n"
			"  RxDropFramesErr: %u\n"
			,
			portname,
			mibc.TxGoodOctets,
			mibc.TxGoodFrames,
			mibc.TxBadOctets,
			mibc.TxBadFrames,
			mibc.TxDropFrames,
			mibc.RxGoodOctets,
			mibc.RxGoodFrames,
			mibc.RxBadOctets,
			mibc.RxBadFrames,
			mibc.RxDropFramesFilter,
			mibc.RxDropFramesErr
			);
#else
		printf("%s MIB counters:\n"
			"  TxGoodFrames: %u\n"
			"  TxBadFrames: %u\n"
			"  RxGoodFrames: %u\n"
			"  RxBadFrames: %u\n"
			,
			portname,
			mibc.TxGoodFrames,
			mibc.TxBadFrames,
			mibc.RxGoodFrames,
			mibc.RxBadFrames
			);
#endif

	return retVal;
}

static int show_vlan_pvid_wan(void)
{
	int retVal;
	unsigned int arg = 2;

	retVal = mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_PVID_WAN_GET, 0, &arg);
	if (retVal != 0)
		arg = 2;

	printf("%d\n", arg);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// MTK_ESW PROCESS
////////////////////////////////////////////////////////////////////////////////

int mtk_esw_node(void)
{
	return mknod(MTK_ESW_DEVPATH, S_IFCHR | 0666, makedev(MTK_ESW_DEVMAJOR, 0));
}

int mtk_esw_main(int argc, char **argv)
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
	case MTK_ESW_IOCTL_STATUS_BYTES_PORT:
		return 1;
	
	case MTK_ESW_IOCTL_STATUS_LINK_PORT:
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
		return show_status_link(cmd, arg);
	
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT:
		return show_status_speed(arg);
	
	case MTK_ESW_IOCTL_STATUS_MIB_PORT:
		return show_mib_counters(arg);
	
	case MTK_ESW_IOCTL_VLAN_PVID_WAN_GET:
		return show_vlan_pvid_wan();
	}

	return mtk_esw_ioctl(cmd, par, &arg);
}

