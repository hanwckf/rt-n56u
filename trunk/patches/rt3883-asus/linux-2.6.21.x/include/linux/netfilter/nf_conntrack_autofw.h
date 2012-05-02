#ifndef _NF_CONNTRACK_AUTOFW_H
#define _NF_CONNTRACK_AUTOFW_H

#ifdef __KERNEL__

struct nf_ct_autofw_master {
	u_int16_t dport[2];     /* Related destination port range */
	u_int16_t to[2];        /* Port range to map related destination port range to */
};


#endif /* __KERNEL__ */

#endif /* _NF_CONNTRACK_AUTOFW_H */
