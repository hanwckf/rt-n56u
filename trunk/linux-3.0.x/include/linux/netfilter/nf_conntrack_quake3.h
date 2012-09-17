#ifndef _IP_CT_QUAKE3
#define _IP_CT_QUAKE3

/* Don't confuse with 27960, often used as the Server Port */
#define QUAKE3_MASTER_PORT 27950

struct quake3_search {
	const char marker[4]; /* always 0xff 0xff 0xff 0xff ? */
	const char *pattern;
	size_t plen;
};

extern unsigned int (*nf_nat_quake3_hook)(struct sk_buff *skb,
				   enum ip_conntrack_info ctinfo,
				   struct nf_conntrack_expect *exp);
#endif /* _IP_CT_QUAKE3 */
