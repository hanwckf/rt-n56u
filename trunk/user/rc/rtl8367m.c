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
#include <sys/ioctl.h>
#include <linux/rtl8367m_drv.h>

#include "rtl8367m.h"

int rtl8367m_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(RTL8367M_DEVPATH, O_RDONLY);
	if (fd < 0) {
		perror(RTL8367M_DEVPATH);
		return errno;
	}

	cmd &= ((1L << RTL8367M_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << RTL8367M_IOCTL_CMD_LENGTH_BITS);

	if(ioctl(fd, cmd, value) < 0)
	{
		perror("ioctl");
		retVal = errno;
	}
	
	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// RTL8367M STATUS
////////////////////////////////////////////////////////////////////////////////

int phy_status_port_link_wan(unsigned int *p_link_on)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_LINK_PORT_WAN, 0, p_link_on);
}

int phy_status_port_link_wan_all(unsigned int *p_link_on)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_LINK_PORTS_WAN, 0, p_link_on);
}

int phy_status_port_link_lan_all(unsigned int *p_link_on)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_LINK_PORTS_LAN, 0, p_link_on);
}

////////////////////////////////////////////////////////////////////////////////

int phy_status_port_speed_wan(unsigned int *p_speed_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_SPEED_PORT_WAN, 0, p_speed_mode);
}

int phy_status_port_speed_lan1(unsigned int *p_speed_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN1, 0, p_speed_mode);
}

int phy_status_port_speed_lan2(unsigned int *p_speed_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN2, 0, p_speed_mode);
}

int phy_status_port_speed_lan3(unsigned int *p_speed_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN3, 0, p_speed_mode);
}

int phy_status_port_speed_lan4(unsigned int *p_speed_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN4, 0, p_speed_mode);
}

////////////////////////////////////////////////////////////////////////////////

int phy_link_port_wan(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return rtl8367m_ioctl(RTL8367M_IOCTL_SPEED_PORT_WAN, 0, &port_control);
}

int phy_link_port_lan1(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return rtl8367m_ioctl(RTL8367M_IOCTL_SPEED_PORT_LAN1, 0, &port_control);
}

int phy_link_port_lan2(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return rtl8367m_ioctl(RTL8367M_IOCTL_SPEED_PORT_LAN2, 0, &port_control);
}

int phy_link_port_lan3(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = (flow_mode << 8) | link_mode;
	return rtl8367m_ioctl(RTL8367M_IOCTL_SPEED_PORT_LAN3, 0, &port_control);
}

int phy_link_port_lan4(unsigned int link_mode, unsigned int flow_mode)
{
	unsigned int port_control = ((flow_mode << 8) | link_mode);
	return rtl8367m_ioctl(RTL8367M_IOCTL_SPEED_PORT_LAN4, 0, &port_control);
}

////////////////////////////////////////////////////////////////////////////////

int phy_storm_unicast_unknown(unsigned int storm_rate_mbps)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STORM_UNICAST_UNK, 0, &storm_rate_mbps);
}

int phy_storm_multicast_unknown(unsigned int storm_rate_mbps)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STORM_MULTICAST_UNK, 0, &storm_rate_mbps);
}

int phy_storm_multicast(unsigned int storm_rate_mbps)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STORM_MULTICAST, 0, &storm_rate_mbps);
}

int phy_storm_broadcast(unsigned int storm_rate_mbps)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_STORM_BROADCAST, 0, &storm_rate_mbps);
}

////////////////////////////////////////////////////////////////////////////////

int phy_led_mode_green(unsigned int led_green)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_LED_MODE_GROUP0, 0, &led_green);
}

int phy_led_mode_yellow(unsigned int led_yellow)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_LED_MODE_GROUP1, 0, &led_yellow);
}

////////////////////////////////////////////////////////////////////////////////

int phy_green_ethernet(unsigned int green_ethernet_on)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_GREEN_ETHERNET, 0, &green_ethernet_on);
}

int phy_jumbo_frames(unsigned int jumbo_frames_on)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_JUMBO_FRAMES, 0, &jumbo_frames_on);
}

////////////////////////////////////////////////////////////////////////////////

int phy_bridge_mode(unsigned int bridge_mode, int isolated_mode)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_BRIDGE_MODE, isolated_mode, &bridge_mode);
}

int phy_vlan_reset_table(void)
{
	unsigned int unused = 0;
	return rtl8367m_ioctl(RTL8367M_IOCTL_VLAN_RESET_TABLE, 0, &unused);
}

int phy_vlan_ingress_mode(unsigned int ingress_enabled)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_VLAN_INGRESS_MODE, 0, &ingress_enabled);
}

int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_VLAN_ACCEPT_PORT_MODE, accept_mode, &port_pask);
}

int phy_vlan_create_entry(int pvid, int priority, unsigned int member, unsigned int untag, int fid)
{
	unsigned int vlan4k_info = (((fid & 0xFF) << 16) | ((priority & 0x07) << 12) | (pvid & 0x0FFF));
	unsigned int vlan4k_mask = (((untag & 0x7F) << 16) | (member & 0x7F));
	
	return rtl8367m_ioctl(RTL8367M_IOCTL_VLAN_CREATE_PORT_VID, vlan4k_info, &vlan4k_mask);
}


////////////////////////////////////////////////////////////////////////////////
// RT3662 GPIO CONTROL
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_mode_set_bit(int idx, unsigned int value)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_MODE_SET_BIT, idx, &value);
}

int cpu_gpio_set_pin_direction(int pin, unsigned int use_output_direction)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_PIN_SET_DIRECTION, pin, &use_output_direction);
}

int cpu_gpio_set_pin(int pin, unsigned int value)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_PIN_SET_VAL, pin, &value);
}

int cpu_gpio_get_pin(int pin, unsigned int *p_value)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_PIN_GET_VAL, pin, p_value);
}

void cpu_gpio_write(int gpionum, int value)
{
	cpu_gpio_set_pin_direction(gpionum, GPIO_DIR_OUT);
	cpu_gpio_set_pin(gpionum, value);
}

////////////////////////////////////////////////////////////////////////////////
// STATUS
////////////////////////////////////////////////////////////////////////////////

int show_usage()
{
	printf("Usage: rtl8367m COMMAND [ARG1] [ARG2]\n"
	" COMMAND:\n"
	"    0 [MODE]        Set GPIO Mode RAW DWORD\n"
	"    1 [0 | 1] [BIT] Set GPIO Mode Bit\n"
	"    2               Show GPIO Mode RAW\n"
	"    3 [0 | 1] [PIN] Set GPIO Pin Direction (0=Input, 1=Output)\n"
	"    4 [0 | 1] [PIN] Set GPIO Pin Value\n"
	"    5 [PIN]         Show GPIO Pin Value\n\n"
	"   10               Show WAN Port Link Status\n"
	"   11               Show WAN Ports Link Status\n"
	"   12               Show LAN Ports Link Status\n\n"
	"   20               Show WAN Port Speed Status\n"
	"   21               Show LAN1 Port Speed Status\n"
	"   22               Show LAN2 Port Speed Status\n"
	"   23               Show LAN3 Port Speed Status\n"
	"   24               Show LAN4 Port Speed Status\n\n"
	"   30               Show WAN Port MIB Counters\n"
	"   31               Show LAN1 Port MIB Counters\n"
	"   32               Show LAN2 Port MIB Counters\n"
	"   33               Show LAN3 Port MIB Counters\n"
	"   34               Show LAN4 Port MIB Counters\n"
	"   35               Show CPU WAN Port MIB Counters\n"
	"   36               Show CPU LAN Port MIB Counters\n"
	"   37               Reset All Ports MIB Counters\n\n"
	"   40 [0x25252525]  Full Reset and Reinit Switch\n\n"
	"   50 [0..8] [0..3] Config WAN Bridge Mode And Isolation\n\n"
	"   60               Reset VLAN Table and Init VLAN1\n"
	"   61 [0 | 1]       Set VLAN Ingress Enabled\n"
	"   62 [MASK] [0..2] Set VLAN Accept Mode for Ports Mask\n"
	"   63 [MASK] [INFO] Create Port-Based VLAN Entry\n\n"
	"   70 [1..1024]     Set Unknown Unicast Storm Rate for All Ports\n"
	"   71 [1..1024]     Set Unknown Multicast Storm Rate for All Ports\n"
	"   72 [1..1024]     Set Multicast Storm Rate for All Ports\n"
	"   73 [1..1024]     Set Broadcast Storm Rate for All Ports\n\n"
	"   75 [1 | 0]       Set Jumbo Frames Accept Disabled\n"
	"   76 [1 | 0]       Set GreenEthernet Disabled\n\n"
	"   80 [0..11]       Set LED Action Group0\n"
	"   81 [0..11]       Set LED Action Group1\n"
	"   82 [0..11]       Set LED Action Group2\n\n"
	"   90 [MODE]        Set WAN Port Link Mode (flow|link)\n"
	"   91 [MODE]        Set LAN1 Port Link Mode (flow|link)\n"
	"   92 [MODE]        Set LAN2 Port Link Mode (flow|link)\n"
	"   93 [MODE]        Set LAN3 Port Link Mode (flow|link)\n"
	"   94 [MODE]        Set LAN4 Port Link Mode (flow|link)\n\n"
	"  100 [0..7]        Set ExtIf RGMII Delay RX\n"
	"  101 [0..1]        Set ExtIf RGMII Delay TX\n"
	);
	return 1;
}

int show_status_gpio_mode()
{
	int retVal;
	unsigned int arg = 0;
	retVal = rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_MODE_GET, 0, &arg);
	if (retVal == 0)
	{
		printf("GPIO Mode = 0x%08X\n", arg);
	}
	
	return retVal;
}

int show_status_gpio_pin(unsigned int par)
{
	int retVal;
	unsigned int arg = 0;
	retVal = rtl8367m_ioctl(RTL8367M_IOCTL_GPIO_PIN_GET_VAL, par, &arg);
	if (retVal == 0)
	{
		printf("GPIO Pin %d = %d\n", par, arg);
	}
	
	return retVal;
}

int show_status_link(unsigned int cmd)
{
	int retVal;
	unsigned int arg = 0;
	char *portname = "";
	retVal = rtl8367m_ioctl(cmd, 0, &arg);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case RTL8367M_IOCTL_STATUS_LINK_PORT_WAN:
			portname = "WAN port";
			break;
		case RTL8367M_IOCTL_STATUS_LINK_PORTS_WAN:
			portname = "WAN ports";
			break;
		case RTL8367M_IOCTL_STATUS_LINK_PORTS_LAN:
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
	retVal = rtl8367m_ioctl(cmd, 0, &arg);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case RTL8367M_IOCTL_STATUS_SPEED_PORT_WAN:
			portname = "WAN port";
			break;
		case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN1:
			portname = "LAN1 port";
			break;
		case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN2:
			portname = "LAN2 port";
			break;
		case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN3:
			portname = "LAN3 port";
			break;
		case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN4:
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
	mib_counters_t *mibc;
	char *portname = "";
	
	mibc = malloc( sizeof(mib_counters_t) );
	if (!mibc) {
		perror("malloc");
		return -1;
	}
	
	memset(mibc, 0, sizeof(mib_counters_t));
	retVal = rtl8367m_ioctl(cmd, 0, (unsigned int *)mibc);
	if (retVal == 0)
	{
		switch (cmd)
		{
		case RTL8367M_IOCTL_STATUS_CNT_PORT_WAN:
			portname = "WAN port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN1:
			portname = "LAN1 port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN2:
			portname = "LAN2 port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN3:
			portname = "LAN3 port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN4:
			portname = "LAN4 port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_WAN:
			portname = "CPU WAN port";
			break;
		case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_LAN:
			portname = "CPU LAN port";
			break;
		}
		
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
			"  dot3InPauseFrames: %u\n"
			"  dot3OutPauseFrames: %u\n"
			"  dot1dTpPortInDiscards: %u\n"
			"  dot1dBasePortDelayExceededDiscards: %u\n"
			,
			portname,
			mibc->ifInOctets,
			mibc->ifInUcastPkts,
			mibc->ifOutOctets,
			mibc->ifOutUcastPkts,
			mibc->ifOutMulticastPkts,
			mibc->ifOutBrocastPkts,
			mibc->etherStatsOctets,
			mibc->etherStatsMcastPkts,
			mibc->etherStatsBcastPkts,
			mibc->etherStatsFragments,
			mibc->etherStatsJabbers,
			mibc->etherStatsDropEvents,
			mibc->etherStatsUndersizePkts,
			mibc->etherStatsOversizePkts,
			mibc->etherStatsCollisions,
			mibc->dot3StatsFCSErrors,
			mibc->dot3StatsSymbolErrors,
			mibc->dot3ControlInUnknownOpcodes,
			mibc->dot3InPauseFrames,
			mibc->dot3OutPauseFrames,
			mibc->dot1dTpPortInDiscards,
			mibc->dot1dBasePortDelayExceededDiscards
			);
	}
	
	free(mibc);
	
	return retVal;
}

unsigned int get_param_int_hex(const char *param)
{
	unsigned int retVal = 0;
	
	/* check HEX */
	if (strlen(param) > 2 && param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))
		retVal = strtoul(param+2, NULL, 16);
	else
		retVal = atoi(param);
	
	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// RTL8367M PROCESS
////////////////////////////////////////////////////////////////////////////////

int rtl8367m_main(int argc, char **argv)
{
	unsigned int cmd, par = 0, arg = 0;
	
	if (argc < 2)
		return show_usage();
	
	cmd = atoi(argv[1]);
	if (argc > 2)
		arg = get_param_int_hex(argv[2]);
	if (argc > 3)
		par = get_param_int_hex(argv[3]);
	
	switch (cmd)
	{
	case RTL8367M_IOCTL_GPIO_MODE_GET:
		return show_status_gpio_mode();
	
	case RTL8367M_IOCTL_GPIO_PIN_GET_VAL:
		return show_status_gpio_pin(arg);
	
	case RTL8367M_IOCTL_STATUS_LINK_PORT_WAN:
	case RTL8367M_IOCTL_STATUS_LINK_PORTS_WAN:
	case RTL8367M_IOCTL_STATUS_LINK_PORTS_LAN:
		return show_status_link(cmd);
	
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_WAN:
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN1:
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN2:
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN3:
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN4:
		return show_status_speed(cmd);
	
	case RTL8367M_IOCTL_STATUS_CNT_PORT_WAN:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN1:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN2:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN3:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN4:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_WAN:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_LAN:
		return show_mib_counters(cmd);
	}
	
	return rtl8367m_ioctl(cmd, par, &arg);
}

