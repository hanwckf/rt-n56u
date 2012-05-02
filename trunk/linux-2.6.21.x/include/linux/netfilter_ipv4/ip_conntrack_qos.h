#ifndef	__IP_CT_QOS_H__
#define	__IP_CT_QOS_H__
	
	//#include <asm/param.h>
	//#include <asm/bitops.h>
	//#include <linux/rtnetlink.h>
	#include <linux/list.h>
	//#include <linux/netfilter_ipv4/listhelp.h>
	#include <linux/in.h>
	#include <linux/netfilter_ipv4/ip_conntrack.h>
	#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
	
	/* For debug */
	#define IPS_QOS_DEBUGMOD				0
	#if		IPS_QOS_DEBUGMOD
		#define IPS_QOSDBG(...)			printk(__VA_ARGS__)
		#define IPS_QOS_ASSERT(...)		/*...Non-finish*/
	#else
		#define IPS_QOSDBG(...)
		#define IPS_QOS_ASSERT(...)
	#endif
	#define IPS_QOS_ERROR					printk("Error at %s- #%d- %s\n"		\
												, __FILE__, __LINE__, __func__)
	#define IPS_QOS_SET_BIT(x, y...);	set_bit(x, ##y);
	#define IPS_QOS_SET_TYPE(x, y...);	IPS_QOS_SET_BIT(x, ##y);		\
												prio=x;
	#define IPS_QOS_TEST_BIT(x, y...)	test_bit(x, ##y)
	
	/* Bitmap to ips_qos_type in ip_conntrack_tuple */
	#define IPS_QOS_HI_BIT					1
	#define IPS_QOS_MID_BIT				2
	#define IPS_QOS_LOW_BIT				3
	#define IPS_QOS_KEEP_BIT				4	//keep original tcp timeout value
	
	/* Priority assigning */
	#define IPS_LAN_WAN						IPS_QOS_MID_BIT
	//#define IPS_WAN_WAN						IPS_QOS_MID_BIT
	#define IPS_LAN_LAN						IPS_QOS_MID_BIT		/*FIXME: WLAN<->LAN*/
	#define IPS_DEV_WAN						IPS_QOS_HI_BIT
	#define IPS_LAN_DEV						IPS_QOS_HI_BIT
	#define IPS_DEV_DEV						IPS_QOS_LOW_BIT
	#define IPS_UNKNOWN_TYPE				IPS_QOS_LOW_BIT
	
	/*  */
	#define IPS_QOS_HI_MAX_LEN			384
	#define IPS_QOS_HI_INC					++ips_qos_hi_len
	#define IPS_QOS_HI_DEC					--ips_qos_hi_len
	#define IPS_QOS_INC						++ips_qos_len
	#define IPS_QOS_DEC						--ips_qos_len
	#define IPS_QOS_DEL						0
	#define IPS_QOS_ADD						1
	
	/* Time for re-checking max len */
	#define IPS_QOS_DELAY					5*60*HZ
	
	/* Value for ip_ct_tcp_timeout_establish */
	#define TCP_TIMEOUT_ESTABLISH_NORMAL	7200	/*sec, normal mode or high priority ct*/
	#define TCP_TIMEOUT_ESTABLISH_BT		5		/*sec, bt mode*/
	#define TCP_TRAFFIC_THROTTLE				1024
	#define TCP_TRAFFIC_LOW					768
	
	/* Filters for IPS_QoS */
	#define IPS_QOS_FILTER_LIST_MAX			4
	#define IPS_QOS_FILTER_ICMP				0	/*IPPROTO_ICMP = 1*/
	#define IPS_QOS_FILTER_IGMP				1	/*IPPROTO_IGMP = 2*/
	#define IPS_QOS_FILTER_TCP				2	/*IPPROTO_TCP = 6*/
	#define IPS_QOS_FILTER_UDP				3	/*IPPROTO_UDP = 17*/
	typedef struct	inet_protocol_ips_qos_filter {
		uint16_t		protonum, 
						reserve;
		union ip_conntrack_manip_proto  u;
		char			name[16];
		struct list_head	list;
	}IPS_QOS_FILTER;
	
	/* Function */
	int		ips_qos_check_entry(struct ip_conntrack_tuple *, int *);
	void	ips_qos_modify_qostype_2_ct(const int, const int, struct ip_conntrack *);
	void	u32to8(uint32_t *, uint8_t **);
	void	tune_tcp_timeout_establish(int);
	void	ips_qos_alter_len(const int, const int, const int);
	void	ips_qos_tune_len(const int, const int, const int);
	int		ips_qos_get_priority(struct ip_conntrack *);
	int		ips_qos_get_keeptimeout(struct ip_conntrack *);
	
	/* Variable */
	extern int		ips_qos_max_len;
	extern int		ips_qos_len;
	extern int		ips_qos_hi_len;
	extern atomic_t	ip_conntrack_amount_atomic;
#endif
