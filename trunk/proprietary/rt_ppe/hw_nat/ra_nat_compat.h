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

#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS) || \
    defined (CONFIG_MT7610_AP_MBSS) || defined (CONFIG_MT76X2_AP_MBSS) || \
    defined (CONFIG_MT76X3_AP_MBSS) || \
    defined (CONFIG_RT2860V2_AP_MBSS)
#define HWNAT_USE_IF_MBSS
#endif

#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT5592_AP_WDS) || defined (CONFIG_RT3593_AP_WDS) || \
    defined (CONFIG_MT7610_AP_WDS) || defined (CONFIG_MT76X2_AP_WDS) || \
    defined (CONFIG_MT76X3_AP_WDS) || \
    defined (CONFIG_RT2860V2_AP_WDS)
#define HWNAT_USE_IF_WDS
#endif

#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI) || \
    defined (CONFIG_MT7610_AP_APCLI) || defined (CONFIG_MT76X2_AP_APCLI) || \
    defined (CONFIG_MT76X3_AP_APCLI) || \
    defined (CONFIG_RT2860V2_AP_APCLI)
#define HWNAT_USE_IF_APCLI
#endif

#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH) || \
    defined (CONFIG_MT7610_AP_MESH) || defined (CONFIG_MT76X2_AP_MESH) || \
    defined (CONFIG_MT76X3_AP_MESH) || \
    defined (CONFIG_RT2860V2_AP_MESH)
#define HWNAT_USE_IF_MESH
#endif

#if defined (CONFIG_CHIP_MT7615E) || defined (CONFIG_CHIP_MT7915)

#if defined (CONFIG_MBSS_SUPPORT)
#define HWNAT_USE_IF_MBSS
#endif

#if defined (CONFIG_WDS_SUPPORT)
#define HWNAT_USE_IF_WDS
#endif

#if defined (CONFIG_APCLI_SUPPORT)
#define HWNAT_USE_IF_APCLI
#endif

#if defined (CONFIG_DBDC_MODE) 
#define HWNAT_USE_IF_DBDC
#endif

#endif

/* old drivers send skb via ra0/rai0 and use skb->cb[CB_OFF+6] for store type */
#if defined (CONFIG_RT_FIRST_IF_RT3090) || defined (CONFIG_RT_FIRST_IF_RT5392) || \
    defined (CONFIG_RT_FIRST_IF_RT5592) || defined (CONFIG_RT_FIRST_IF_RT3593) || \
    defined (CONFIG_RT_FIRST_IF_MT7610E) || defined (CONFIG_RT_FIRST_IF_RT2860)
#define HWNAT_USE_FIRST_IF_CBOFF
#endif

#if defined (CONFIG_RT_SECOND_IF_RT3090) || defined (CONFIG_RT_SECOND_IF_RT5392) || \
    defined (CONFIG_RT_SECOND_IF_RT5592) || defined (CONFIG_RT_SECOND_IF_RT3593) || \
    defined (CONFIG_RT_SECOND_IF_MT7610E)
#define HWNAT_USE_SECOND_IF_CBOFF
#endif


#endif
