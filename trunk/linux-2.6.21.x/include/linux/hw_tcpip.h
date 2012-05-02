#ifndef __HW_TCPIP
#define __HW_TCPIP

#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/sock.h>
#include <net/ip_fib.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_nat.h>

struct hw_tcpip_helpers {
	struct timer_list       timer;
	atomic_t                refcnt;
	unsigned long           summy;
	void (*refresh_tcp_connection)(struct tcphdr *tcph,struct ip_conntrack *conntrack);
	void (*del_hw_fib_entry)(struct fib_info *fi);
	void (*add_hw_fib_entry)(struct fib_table *tb, struct rtmsg *r, struct kern_rta *rta, struct fib_info *fi);
	int (*del_hw_nat_entry)(struct ip_conntrack *conn);
	void (*add_hw_SNAT_entry)(struct ip_nat_info *info,struct ip_conntrack *conntrack,int hooknum);
	void (*add_hw_DNAT_entry)(struct ip_nat_info *info,struct ip_conntrack *conntrack,int hooknum);
	int (*check_hw_reference)(struct neighbour *neigh);
	int (*del_hw_arp_entry)(struct neighbour *neigh);
	void (*add_hw_arp_entry)(struct neighbour *neigh, const u8 *lladdr);
	void (*del_hw_pst_entry)(unsigned short sid);
#if defined(CONFIG_SOFTWARE_TURBO) || defined(CONFIG_SOFTWARE_TURBO_MODULE)
	int (*sw_net_rx_action)(struct sk_buff *skb);
	void (*sw_drop)(struct ip_conntrack *conn,struct sk_buff *skb);
	void (*sw_ipt_changed)(void);
	void (*sw_refresh_out_dev)(void *sw, struct net_device *odev,struct net_device *ndev);
#endif
};

//struct hw_tcpip_helpers * hw_tcpip = NULL;
extern struct hw_tcpip_helpers *hw_tcpip;

extern int register_hw_tcpip(struct hw_tcpip_helpers *tcpip);
extern void unregister_hw_tcpip(struct hw_tcpip_helpers *tcpip);
extern int sw_disable_80;
#endif
