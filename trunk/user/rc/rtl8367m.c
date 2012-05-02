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
	return rtl8367m_ioctl(RTL8367M_IOCTL_LED_MODE_GREEN, 0, &led_green);
}

int phy_led_mode_yellow(unsigned int led_yellow)
{
	return rtl8367m_ioctl(RTL8367M_IOCTL_LED_MODE_YELLOW, 0, &led_yellow);
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
	unsigned int vlan4k_mask = (((untag & 0x03FF) << 16) | (member & 0x03FF));
	
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
// RTL8367M PROCESS
////////////////////////////////////////////////////////////////////////////////

int rtl8367m_main(int argc, char **argv)
{
	unsigned int cmd, par = 0, arg = 0;

	if (argc < 3)
	{
		return 0;
	}

	cmd = atoi(argv[1]);

	// skip IOCTL with sizeof(arg) > 4
	switch (cmd)
	{
	case RTL8367M_IOCTL_STATUS_CNT_PORT_WAN:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN1:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN2:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN3:
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN4:
		return -1;
	}

	if (argc > 2)
		arg = atoi(argv[2]);
	if (argc > 3)
		par = atoi(argv[3]);

	return rtl8367m_ioctl(cmd, par, &arg);
}

