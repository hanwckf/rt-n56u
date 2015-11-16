#include <linux/export.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_fib.h>
#include <net/addrconf.h>

static u32 __ipv6_select_ident(u32 hashrnd, struct in6_addr *dst, struct in6_addr *src)
{
	u32 hash, id;

	hash = __ipv6_addr_jhash(dst, hashrnd);
	hash = __ipv6_addr_jhash(src, hash);

	/* Treat id of 0 as unset and if we get 0 back from ip_idents_reserve,
	 * set the hight order instead thus minimizing possible future
	 * collisions.
	 */
	id = ip_idents_reserve(hash, 1);
	if (unlikely(!id))
		id = 1 << 31;

	return id;
}

/* This function exists only for tap drivers that must support broken
 * clients requesting UFO without specifying an IPv6 fragment ID.
 *
 * This is similar to ipv6_select_ident() but we use an independent hash
 * seed to limit information leakage.
 */
void ipv6_proxy_select_ident(struct sk_buff *skb)
{
	static u32 ip6_proxy_idents_hashrnd __read_mostly;
	struct in6_addr buf[2];
	struct in6_addr *addrs;
	u32 id;

	addrs = skb_header_pointer(skb,
				   skb_network_offset(skb) +
				   offsetof(struct ipv6hdr, saddr),
				   sizeof(buf), buf);
	if (!addrs)
		return;

	net_get_random_once(&ip6_proxy_idents_hashrnd,
			    sizeof(ip6_proxy_idents_hashrnd));

	id = __ipv6_select_ident(ip6_proxy_idents_hashrnd,
				 &addrs[1], &addrs[0]);
	skb_shinfo(skb)->ip6_frag_id = htonl(id);
}
EXPORT_SYMBOL_GPL(ipv6_proxy_select_ident);

void ipv6_select_ident(struct frag_hdr *fhdr, struct rt6_info *rt)
{
	static u32 ip6_idents_hashrnd __read_mostly;
	u32 id;

	net_get_random_once(&ip6_idents_hashrnd, sizeof(ip6_idents_hashrnd));

	id = __ipv6_select_ident(ip6_idents_hashrnd, &rt->rt6i_dst.addr,
				 &rt->rt6i_src.addr);
	fhdr->identification = htonl(id);
}
EXPORT_SYMBOL(ipv6_select_ident);
