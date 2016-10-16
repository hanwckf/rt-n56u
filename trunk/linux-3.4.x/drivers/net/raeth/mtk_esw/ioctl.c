/*
 * switch control IOCTL implementation (common code)
 */

////////////////////////////////////////////////////////////////////////////////////

static void reset_and_init_switch(void)
{
	u32 i, port_phy_power[ESW_EPHY_ID_MAX+1];

	/* store PHY power state before down */
	memcpy(port_phy_power, g_port_phy_power, sizeof(g_port_phy_power));

	power_down_all_phy();

	esw_soft_reset();
	esw_ioctl_init_post();

	/* restore PHY power state */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (port_phy_power[i])
			esw_port_phy_power(i, 1, 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////

static void fill_bridge_members(void)
{
#if defined (CONFIG_MT7530_GSW)
	set_bit(1, g_vlan_pool);
	set_bit(2, g_vlan_pool); // VID2 filled on U-Boot
#endif
	memset(g_bwan_member, 0, sizeof(g_bwan_member));

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN2][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN2][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN4][LAN_PORT_4].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN4][LAN_PORT_4].rule = SWAPI_VLAN_RULE_WAN_LAN4;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_4].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_4].rule = SWAPI_VLAN_RULE_WAN_LAN4;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;
}

////////////////////////////////////////////////////////////////////////////////////

static long mtk_esw_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;
	port_bytes_t port_bytes = {0};
	esw_mib_counters_t port_counters;

	unsigned int uint_param = (req >> MTK_ESW_IOCTL_CMD_LENGTH_BITS);
	req &= ((1u << MTK_ESW_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&esw_access_mutex);

	switch(req)
	{
	case MTK_ESW_IOCTL_STATUS_LINK_PORT:
		uint_result = esw_status_link_port_uapi(uint_param);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
		uint_result = esw_status_link_ports(1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
		uint_result = esw_status_link_ports(0);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_CHANGED:
		uint_result = esw_status_link_changed();
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case MTK_ESW_IOCTL_STATUS_SPEED_PORT:
		uint_result = esw_status_speed_port_uapi(uint_param);
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case MTK_ESW_IOCTL_STATUS_BYTES_PORT:
		ioctl_result = esw_status_bytes_port_uapi(uint_param, &port_bytes);
		copy_to_user((port_bytes_t __user *)arg, &port_bytes, sizeof(port_bytes_t));
		break;

	case MTK_ESW_IOCTL_STATUS_MIB_PORT:
		ioctl_result = esw_status_mib_port_uapi(uint_param, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_MIB_RESET_ALL:
		esw_status_mib_reset();
		break;

	case MTK_ESW_IOCTL_RESET_SWITCH:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == SWAPI_MAGIC_RESET_ASIC)
			reset_and_init_switch();
		break;
	case MTK_ESW_IOCTL_PORTS_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_ports_power(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_PORTS_WAN_LAN_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_wan_lan_ports_power(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_MAC_TABLE_CLEAR:
		esw_mac_table_clear();
		break;

	case MTK_ESW_IOCTL_BRIDGE_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_bridge_mode(uint_param, uint_value);
		break;
#if defined (CONFIG_MT7530_GSW)
	case MTK_ESW_IOCTL_PORT_FORWARD_MASK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = override_port_matrix_uapi(uint_param, uint_value);
		break;
#endif

	case MTK_ESW_IOCTL_VLAN_RESET_TABLE:
		esw_vlan_reset_table();
		break;
	case MTK_ESW_IOCTL_VLAN_PVID_WAN_GET:
		uint_result = g_vlan_pvid_wan_untagged;
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_VLAN_ACCEPT_PORT_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_accept_port_mode(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_VLAN_CREATE_PORT_VID:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_create_entry(uint_param, uint_value, 1);
		break;
	case MTK_ESW_IOCTL_VLAN_CREATE_ENTRY:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_create_entry(uint_param, uint_value, 0);
		break;
	case MTK_ESW_IOCTL_VLAN_RULE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_vlan_rule(uint_param, uint_value);
		break;

	case MTK_ESW_IOCTL_STORM_BROADCAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_storm_control_broadcast(uint_value);
		break;

	case MTK_ESW_IOCTL_JUMBO_FRAMES:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_jumbo_frames_accept(uint_value);
		break;

	case MTK_ESW_IOCTL_EEE_LPI:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_eee_lpi_mode(uint_value);
		break;

	case MTK_ESW_IOCTL_IGMP_SNOOPING:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_igmp_snooping_control(uint_value);
		break;
	case MTK_ESW_IOCTL_IGMP_STATIC_PORTS:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_igmp_static_ports(uint_value);
		break;

	case MTK_ESW_IOCTL_LED_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_led_mode(uint_value);
		break;

	case MTK_ESW_IOCTL_SPEED_PORT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_port_link_mode_uapi(uint_param, uint_value);
		break;

	default:
		ioctl_result = -ENOIOCTLCMD;
	}

	mutex_unlock(&esw_access_mutex);

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////////

static int mtk_esw_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static int mtk_esw_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations mtk_esw_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= mtk_esw_ioctl,
	.open		= mtk_esw_open,
	.release	= mtk_esw_release,
};

////////////////////////////////////////////////////////////////////////////////////

int esw_ioctl_init(void)
{
	int r;

	fill_bridge_members();

	r = register_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME, &mtk_esw_fops);
	if (r < 0) {
		printk(KERN_ERR MTK_ESW_DEVNAME ": unable to register character device\n");
		return r;
	}

	esw_link_status_hook = esw_link_status_changed;

#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	igmp_sn_init();
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

void esw_ioctl_uninit(void)
{
	esw_link_status_hook = NULL;

#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	igmp_sn_uninit();
#endif

	unregister_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME);
}

