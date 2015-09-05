#ifndef __RA_NAT_COMPAT_H__
#define __RA_NAT_COMPAT_H__

///////////////////////////////////////////////////////////////

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
#define skb_vlan_tag_present(x)			vlan_tx_tag_present(x)
#define skb_vlan_tag_get(x)			vlan_tx_tag_get(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define vlan_insert_tag_set_proto(x,y,z)	__vlan_put_tag(x,z)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
#define vlan_insert_tag_set_proto(x,y,z)	__vlan_put_tag(x,y,z)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define vlan_insert_tag_hwaccel(x,y,z)		__vlan_hwaccel_put_tag(x,z)
#else
#define vlan_insert_tag_hwaccel(x,y,z)		__vlan_hwaccel_put_tag(x,y,z)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#define ra_dev_get_by_name(x)			dev_get_by_name(x)
#else
#define ra_dev_get_by_name(x)			dev_get_by_name(&init_net,x)
#endif

///////////////////////////////////////////////////////////////

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
#define DEFAULT_UDP_OFFLOAD	1
#else
#define DEFAULT_UDP_OFFLOAD	0
#endif

#if defined (CONFIG_RALINK_MT7621)
#define RAETH_QDMA		/* QoS DMA Engine */
#define RAETH_HW_VLAN4K		/* FE support VLAN 0..4095 */
#endif

///////////////////////////////////////////////////////////////

#endif
