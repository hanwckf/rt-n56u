#ifndef _IP_CT_QUAKE3
#define _IP_CT_QUAKE3

/* Don't confuse with 27960, often used as the Server Port */
#define QUAKE3_MASTER_PORT 27950

struct quake3_search {
	const char marker[4]; /* always 0xff 0xff 0xff 0xff ? */
	const char *pattern;
	size_t plen;
}; 

/* This structure is per expected connection */
struct ip_ct_quake3_expect {
};

/* This structure exists only once per master */
struct ip_ct_quake3_master {
};

extern unsigned int (*ip_nat_quake3_hook)(struct ip_conntrack_expect *exp);
#endif /* _IP_CT_QUAKE3 */
