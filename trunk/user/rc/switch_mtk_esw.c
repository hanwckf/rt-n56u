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

#include "rc.h"
#include "switch.h"

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

int phy_status_port_link_wan(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN, 0, p_link_on);
}

int phy_status_port_link_lan1(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN1, 0, p_link_on);
}

int phy_status_port_link_lan2(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN2, 0, p_link_on);
}

int phy_status_port_link_lan3(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN3, 0, p_link_on);
}

int phy_status_port_link_lan4(unsigned int *p_link_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN4, 0, p_link_on);
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

int phy_status_port_speed_wan(unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN, 0, p_speed_mode);
}

int phy_status_port_speed_lan1(unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1, 0, p_speed_mode);
}

int phy_status_port_speed_lan2(unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2, 0, p_speed_mode);
}

int phy_status_port_speed_lan3(unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3, 0, p_speed_mode);
}

int phy_status_port_speed_lan4(unsigned int *p_speed_mode)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4, 0, p_speed_mode);
}

////////////////////////////////////////////////////////////////////////////////

int phy_link_port_wan(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT_WAN, 0, &port_control);
}

int phy_link_port_lan1(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT_LAN1, 0, &port_control);
}

int phy_link_port_lan2(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT_LAN2, 0, &port_control);
}

int phy_link_port_lan3(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT_LAN3, 0, &port_control);
}

int phy_link_port_lan4(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = ((flow_mode << 8) | link_mode);
	return mtk_esw_ioctl(MTK_ESW_IOCTL_SPEED_PORT_LAN4, 0, &port_control);
}

////////////////////////////////////////////////////////////////////////////////

inline int phy_storm_unicast_unknown(unsigned int storm_rate_mbps)
{
	// N.A.
	return 0;
}

int phy_storm_multicast_unknown(unsigned int storm_rate_mbps)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_STORM_MULTICAST_UNK, 0, &storm_rate_mbps);
}

inline int phy_storm_multicast(unsigned int storm_rate_mbps)
{
	// N.A.
	return 0;
}

inline int phy_storm_broadcast(unsigned int storm_rate_mbps)
{
	// N.A. (brodcast storm also set by unknown multicast)
	return 0;
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

int phy_jumbo_frames(unsigned int jumbo_frames_on)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_JUMBO_FRAMES, 0, &jumbo_frames_on);
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
				   SWAPI_PORTMASK_WAN);
	return mtk_esw_ioctl(MTK_ESW_IOCTL_PORT_POWER, power_on, &ports_mask);
}

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

inline int phy_vlan_ingress_mode(unsigned int port_pask)
{
	// N.A.
	return 0;
}

int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_ACCEPT_PORT_MODE, accept_mode, &port_pask);
}

int phy_vlan_create_port_vid(int pvid, int priority, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0xFF) << 16) | ((priority & 0x07) << 12) | (pvid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFF) << 16) | (member & 0xFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_CREATE_PORT_VID, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_create_entry(int vid, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0xFF) << 16) | (vid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0xFF) << 16) | (member & 0xFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_CREATE_ENTRY, vlan4k_info, &vlan4k_mask);
}

int phy_vlan_rule_set(unsigned int rule_id, int vid, int priority, int tagged)
{
	unsigned int vlan_rule = (((tagged & 0xFF) << 24) | ((priority & 0xFF) << 16) | (vid & 0xFFFF));

	return mtk_esw_ioctl(MTK_ESW_IOCTL_VLAN_RULE_SET, rule_id, &vlan_rule);
}

////////////////////////////////////////////////////////////////////////////////
// RALINK CPU GPIO CONTROL
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_mode_set_bit(int idx, unsigned int value)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_MODE_SET_BIT, idx, &value);
}

int cpu_gpio_set_pin_direction(int pin, unsigned int use_output_direction)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_PIN_SET_DIRECTION, pin, &use_output_direction);
}

int cpu_gpio_set_pin(int pin, unsigned int value)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_PIN_SET_VAL, pin, &value);
}

int cpu_gpio_get_pin(int pin, unsigned int *p_value)
{
	return mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_PIN_GET_VAL, pin, p_value);
}

////////////////////////////////////////////////////////////////////////////////
// STATUS
////////////////////////////////////////////////////////////////////////////////

int show_usage(char *cmd)
{
	printf("Usage: %s COMMAND [ARG1] [ARG2]\n"
	" COMMAND:\n"
	"    0 [MODE]        Set GPIO mode RAW DWORD\n"
	"    1 [0|1] [BIT]   Set GPIO mode bit\n"
	"    2               Show GPIO mode RAW\n"
	"    3 [0|1] [PIN]   Set GPIO pin direction (0=Input, 1=Output)\n"
	"    4 [0|1] [PIN]   Set GPIO pin value\n"
	"    5 [PIN]         Show GPIO pin value\n\n"
	"   10               Show WAN port link status\n"
	"   11               Show WAN ports link status\n"
	"   12               Show LAN ports link status\n"
	"   13               Show LAN1 port link status\n"
	"   14               Show LAN2 port link status\n"
	"   15               Show LAN3 port link status\n"
	"   16               Show LAN4 port link status\n\n"
	"   20               Show WAN port speed status\n"
	"   21               Show LAN1 port speed status\n"
	"   22               Show LAN2 port speed status\n"
	"   23               Show LAN3 port speed status\n"
	"   24               Show LAN4 port speed status\n\n"
	"   30               Show WAN port MIB counters\n"
	"   31               Show LAN1 port MIB counters\n"
	"   32               Show LAN2 port MIB counters\n"
	"   33               Show LAN3 port MIB counters\n"
	"   34               Show LAN4 port MIB counters\n"
	"   36               Show CPU LAN port MIB counters\n\n"
	"   40 [0x25252525]  Full reset and reinit switch\n"
	"   41 [MASK] [0|1]  Set power for ports mask\n"
	"   42               Clear switch MAC table\n\n"
	"   50 [0..8] [0..3] Config WAN bridge mode and isolation\n"
	"   60               Reset VLAN table and init VLAN1\n"
	"   62 [MASK] [0..2] Set VLAN accept mode for ports mask\n"
	"   63 [MASK] [DATA] Create port-based VLAN entry\n"
	"   64 [MASK] [DATA] Create VLAN entry\n\n"
	"   71 [0..1000]     Set Unknown Multicast and Broadcast storm rate for all PHY ports\n"
	"   75 [1|0]         Set Jumbo Frames accept on/off\n"
	"   78 [1|0]         Set IGMP/MLD snooping on/off\n"
	"   80 [7,11]        Set EPHY LED action\n"
	"   90 [MODE]        Set WAN port link mode (flow|link)\n"
	"   91 [MODE]        Set LAN1 port link mode (flow|link)\n"
	"   92 [MODE]        Set LAN2 port link mode (flow|link)\n"
	"   93 [MODE]        Set LAN3 port link mode (flow|link)\n"
	"   94 [MODE]        Set LAN4 port link mode (flow|link)\n\n"
	, cmd);

	return 1;
}

int show_status_gpio_mode(void)
{
	int retVal;
	unsigned int arg = 0;

	retVal = mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_MODE_GET, 0, &arg);
	if (retVal == 0)
		printf("GPIO Mode = 0x%08X\n", arg);

	return retVal;
}

int show_status_gpio_pin(unsigned int par)
{
	int retVal;
	unsigned int arg = 0;

	retVal = mtk_esw_ioctl(MTK_ESW_IOCTL_GPIO_PIN_GET_VAL, par, &arg);
	if (retVal == 0)
		printf("GPIO Pin %d = %d\n", par, arg);

	return retVal;
}

int show_status_link(unsigned int cmd)
{
	int retVal;
	unsigned int arg = 0;
	char *portname = "";

	retVal = mtk_esw_ioctl(cmd, 0, &arg);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN:
			portname = "WAN port";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN1:
			portname = "LAN1 port";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN2:
			portname = "LAN2 port";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN3:
			portname = "LAN3 port";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN4:
			portname = "LAN4 port";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
			portname = "WAN ports";
			break;
		case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
			portname = "LAN ports";
			break;
		}
		
		printf("%s link state: %d\n", portname, arg);
	}

	return retVal;
}

int show_status_speed(unsigned int cmd)
{
	int retVal;
	unsigned int arg = 0;
	int lspeed;
	char *portname = "";
	char lstatus[32];

	retVal = mtk_esw_ioctl(cmd, 0, &arg);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN:
			portname = "WAN port";
			break;
		case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1:
			portname = "LAN1 port";
			break;
		case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2:
			portname = "LAN2 port";
			break;
		case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3:
			portname = "LAN3 port";
			break;
		case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4:
			portname = "LAN4 port";
			break;
		}
		
		if ((arg >> 16) & 0x01)
		{
			switch (arg & 0x03)
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
			
			sprintf(lstatus, "link: %d %s", lspeed, ((arg >> 8) & 0x01) ? "FD" : "HD" );
		}
		else
		{
			sprintf(lstatus, "link: %s", "NO");
		}
		
		printf("%s %s\n", portname, lstatus);
	}

	return retVal;
}

int show_mib_counters(unsigned int cmd)
{
	int retVal;
	arl_mib_counters_t mibc;
	char *portname = "";

	memset(&mibc, 0, sizeof(arl_mib_counters_t));
	retVal = mtk_esw_ioctl(cmd, 0, (unsigned int *)&mibc);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN:
			portname = "WAN port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1:
			portname = "LAN1 port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2:
			portname = "LAN2 port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3:
			portname = "LAN3 port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4:
			portname = "LAN4 port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_WAN:
			portname = "CPU WAN port";
			break;
		case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_LAN:
			portname = "CPU LAN port";
			break;
		}
		
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
			"  RxDropFramesLimiter: %u\n"
			"  RxDropFramesIngress: %u\n"
			"  RxDropFramesControl: %u\n"
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
			mibc.RxDropFramesLimiter,
			mibc.RxDropFramesIngress,
			mibc.RxDropFramesControl
			);
	}

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

	cmd = atoi(argv[1]);
	if (!cmd && argc < 3)
		return show_usage(argv[0]);

	if (argc > 2)
		arg = get_param_int_hex(argv[2]);
	if (argc > 3)
		par = get_param_int_hex(argv[3]);

	switch (cmd)
	{
	case MTK_ESW_IOCTL_GPIO_MODE_GET:
		return show_status_gpio_mode();
	
	case MTK_ESW_IOCTL_GPIO_PIN_GET_VAL:
		return show_status_gpio_pin(arg);
	
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN:
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN1:
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN2:
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN3:
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN4:
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
		return show_status_link(cmd);
	
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN:
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1:
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2:
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3:
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4:
		return show_status_speed(cmd);
	
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_WAN:
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_LAN:
		return show_mib_counters(cmd);
	}

	return mtk_esw_ioctl(cmd, par, &arg);
}

