#ifndef __IP_SET_COMPAT_H
#define __IP_SET_COMPAT_H

/* Not everything could be moved here. Compatibility stuffs can be found in
 * xt_set.c, ip_set_core.c, ip_set_getport.c, pfxlen.c too.
 */

#include <linux/version.h>
#include <linux/netlink.h>

#ifndef rcu_dereference_bh
#define rcu_dereference_bh(p)		rcu_dereference(p)
#endif

#ifndef rcu_dereference_protected
#define rcu_dereference_protected(p, c)	rcu_dereference(p)
#endif

#ifndef __rcu
#define	__rcu
#endif

#ifdef CHECK_KCONFIG
#ifndef CONFIG_SPARSE_RCU_POINTER
#error "CONFIG_SPARSE_RCU_POINTER must be enabled"
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
#define xt_action_param		xt_match_param
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
#define vzalloc(size)		__vmalloc(size,\
					  GFP_KERNEL|__GFP_ZERO|__GFP_HIGHMEM,\
					  PAGE_KERNEL)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
#include <linux/etherdevice.h>

static inline bool ether_addr_equal(const u8 *addr1, const u8 *addr2)
{
	return !compare_ether_addr(addr1, addr2);
}

static inline int nla_put_be64(struct sk_buff *skb, int attrtype, __be64 value)
{
	return nla_put(skb, attrtype, sizeof(__be64), &value);
}

static inline int nla_put_net64(struct sk_buff *skb, int attrtype, __be64 value)
{
	return nla_put_be64(skb, attrtype | NLA_F_NET_BYTEORDER, value);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
#define NETLINK_PORTID(skb)	NETLINK_CB(skb).pid
#else
#define NETLINK_PORTID(skb)	NETLINK_CB(skb).portid
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
#define ns_capable(ns, cap)	capable(cap)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
#define lock_nfnl()		nfnl_lock()
#define unlock_nfnl()		nfnl_unlock()
#else
#define lock_nfnl()		nfnl_lock(NFNL_SUBSYS_IPSET)
#define unlock_nfnl()		nfnl_unlock(NFNL_SUBSYS_IPSET)
#endif

#ifdef NLA_PUT_NET16
static inline int nla_put_be16(struct sk_buff *skb, int attrtype, __be16 value)
{
	return nla_put(skb, attrtype, sizeof(__be16), &value);
}

static inline int nla_put_net16(struct sk_buff *skb, int attrtype, __be16 value)
{
	return nla_put_be16(skb, attrtype | NLA_F_NET_BYTEORDER, value);
}

static inline int nla_put_be32(struct sk_buff *skb, int attrtype, __be32 value)
{
	return nla_put(skb, attrtype, sizeof(__be32), &value);
}

static inline int nla_put_net32(struct sk_buff *skb, int attrtype, __be32 value)
{
	return nla_put_be32(skb, attrtype | NLA_F_NET_BYTEORDER, value);
}
#endif

#endif /* __IP_SET_COMPAT_H */
