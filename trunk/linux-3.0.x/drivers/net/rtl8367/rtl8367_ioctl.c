
static long rtl8367_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;

	rtk_port_linkStatus_t port_link = 0;
	rtk_stat_port_cntr_t  port_counters;
	port_bytes_t port_bytes = {0};

	unsigned int uint_param = (req >> RTL8367_IOCTL_CMD_LENGTH_BITS);
	req &= ((1u << RTL8367_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&asic_access_mutex);

	switch(req)
	{
	case RTL8367_IOCTL_STATUS_LINK_PORT:
		ioctl_result = asic_status_link_port_uapi(uint_param, &port_link);
		put_user(port_link, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_WAN:
		ioctl_result = asic_status_link_ports(1, &port_link);
		put_user(port_link, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_LAN:
		ioctl_result = asic_status_link_ports(0, &port_link);
		put_user(port_link, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_STATUS_LINK_CHANGED:
		uint_result = asic_status_link_changed();
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case RTL8367_IOCTL_STATUS_SPEED_PORT:
		ioctl_result = asic_status_speed_port_uapi(uint_param, &uint_result);
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case RTL8367_IOCTL_STATUS_BYTES_PORT:
		ioctl_result = asic_status_bytes_port_uapi(uint_param, &port_bytes);
		copy_to_user((port_bytes_t __user *)arg, &port_bytes, sizeof(port_bytes_t));
		break;

	case RTL8367_IOCTL_STATUS_MIB_PORT:
		ioctl_result = asic_status_mib_port_uapi(uint_param, &port_counters);
		copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		break;
	case RTL8367_IOCTL_STATUS_MIB_RESET_ALL:
		asic_reset_mib_all();
		break;

	case RTL8367_IOCTL_RESET_ASIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == SWAPI_MAGIC_RESET_ASIC)
			reset_and_init_switch(0);
		break;
	case RTL8367_IOCTL_PORTS_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_ports_power(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_PORTS_WAN_LAN_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_wan_lan_ports_power(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_MAC_TABLE_CLEAR:
		asic_clear_mac_table();
		break;

	case RTL8367_IOCTL_BRIDGE_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_bridge_mode(uint_param, uint_value);
		break;
#if defined(EXT_PORT_INIC)
	case RTL8367_IOCTL_ISOLATE_INIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		toggle_isolation_inic(uint_value);
		break;
	case RTL8367_IOCTL_DISABLE_INIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		toggle_disable_inic(uint_value);
		break;
#endif
	case RTL8367_IOCTL_PORT_FORWARD_MASK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = asic_port_forward_mask(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_VLAN_RESET_TABLE:
		asic_vlan_reset_table();
		break;
	case RTL8367_IOCTL_VLAN_PVID_WAN_GET:
		uint_result = g_vlan_pvid_wan_untagged;
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_VLAN_ACCEPT_PORT_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_accept_port_mode(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_VLAN_CREATE_PORT_VID:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_create_entry(uint_param, uint_value, 1);
		break;
	case RTL8367_IOCTL_VLAN_CREATE_ENTRY:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_create_entry(uint_param, uint_value, 0);
		break;
	case RTL8367_IOCTL_VLAN_RULE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_vlan_rule(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_STORM_UNICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_unicast_unknown(uint_value);
		break;
	case RTL8367_IOCTL_STORM_MULTICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast_unknown(uint_value);
		break;
	case RTL8367_IOCTL_STORM_MULTICAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast(uint_value);
		break;
	case RTL8367_IOCTL_STORM_BROADCAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_broadcast(uint_value);
		break;

	case RTL8367_IOCTL_JUMBO_FRAMES:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_jumbo_frames_accept(uint_value);
		break;

	case RTL8367_IOCTL_GREEN_ETHERNET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_green_ethernet_mode(uint_value);
		break;
	case RTL8367_IOCTL_EEE_LPI:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_eee_lpi_mode(uint_value);
		break;

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	case RTL8367_IOCTL_IGMP_SNOOPING:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		igmp_sn_set_enable(uint_value);
		break;
	case RTL8367_IOCTL_IGMP_STATIC_PORTS:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		igmp_sn_set_static_ports(uint_value);
		break;
#endif

	case RTL8367_IOCTL_LED_MODE_GROUP0:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group0(uint_value);
		break;
	case RTL8367_IOCTL_LED_MODE_GROUP1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group1(uint_value);
		break;
	case RTL8367_IOCTL_LED_MODE_GROUP2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group2(uint_value);
		break;

	case RTL8367_IOCTL_SPEED_PORT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_port_link_mode_uapi(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_RGMII_DELAY_RX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_rx(uint_value);
		break;

	case RTL8367_IOCTL_RGMII_DELAY_TX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_tx(uint_value);
		break;

	case RTL8367_IOCTL_UCAST_MAC_DUMP:
		asic_dump_ucast_table();
		break;
#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	case RTL8367_IOCTL_MCAST_LUT_DUMP:
		igmp_sn_dump_mcast_table();
		break;
#endif
	case RTL8367_IOCTL_ISOLATION_DUMP:
		asic_dump_isolation();
		break;

	case RTL8367_IOCTL_REG_GET:
		asic_reg_get(uint_param, &uint_result);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_REG_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_reg_set(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_PHY_REG_GET:
		asic_phy_reg_get((uint_param >> 8) & 0x1f, uint_param & 0x1f, &uint_result);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_PHY_REG_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_phy_reg_set((uint_param >> 8) & 0x1f, uint_param & 0x1f, uint_value);
		break;

	default:
		ioctl_result = -ENOIOCTLCMD;
	}

	mutex_unlock(&asic_access_mutex);

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////////

static int rtl8367_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static int rtl8367_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations rtl8367_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= rtl8367_ioctl,
	.open		= rtl8367_open,
	.release	= rtl8367_release,
};

