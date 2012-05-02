#ifndef _IP_CONNTRACK_IKE_H
#define _IP_CONNTRACK_IKE_H
/* IKE tracking. */
#define IKE_PORT	500

struct ike_pkt_hdr {
	u_int32_t ocookieh;
	u_int32_t ocookiel;
	u_int32_t rcookieh;
	u_int32_t rcookiel;
};

struct ip_ct_ike_db {
	struct list_head list;
	u_int32_t ocookieh;
	u_int32_t ocookiel;
	u_int32_t rcookieh;
	u_int32_t rcookiel;
	u_int32_t src;
	u_int32_t dst;
	u_int16_t sport;
	u_int16_t dport;
	u_int16_t aliasport;
	u_int16_t dummy;
};

extern struct list_head ike_db_list;
extern rwlock_t ip_ike_lock;

extern void ip_conntrack_find_ike_db_get(u_int32_t ocookieh,u_int32_t ocookiel,
		u_int32_t rcookieh, u_int32_t rcookiel, u_int32_t src, u_int32_t dst, 
		u_int16_t sport, u_int16_t dport, u_int16_t *aliasport, u_int16_t *direction, u_int16_t *state, u_int16_t *find);

extern inline int ike_find_ocookie_cmp_fn(const struct ip_ct_ike_db *i,
		    u_int32_t ocookieh,u_int32_t ocookiel);

extern void ip_conntrack_udp_destroy_ike_db(struct ip_conntrack *ct);

inline int ike_find_entry_cmp_fn(const struct ip_ct_ike_db *i, u_int32_t src,u_int32_t dst)
{
	return i->src == src && i->dst == dst;
}

inline int ike_find_ocookie_cmp_fn(const struct ip_ct_ike_db *i, u_int32_t ocookieh,u_int32_t ocookiel)
{
	return i->ocookieh == ocookieh && i->ocookiel == ocookiel;
}

#endif /* _IP_CONNTRACK_IKE_H */
