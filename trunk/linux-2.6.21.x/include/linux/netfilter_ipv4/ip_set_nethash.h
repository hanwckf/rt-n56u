#ifndef __IP_SET_NETHASH_H
#define __IP_SET_NETHASH_H

#include <linux/netfilter_ipv4/ip_set.h>

#define SETTYPE_NAME "nethash"
#define MAX_RANGE 0x0000FFFF

struct ip_set_nethash {
	ip_set_ip_t *members;		/* the nethash proper */
	uint32_t initval;		/* initval for jhash_1word */
	uint32_t prime;			/* prime for double hashing */
	uint32_t hashsize;		/* hash size */
	uint16_t probes;		/* max number of probes  */
	uint16_t resize;		/* resize factor in percent */
	unsigned char cidr[30];		/* CIDR sizes */
};

struct ip_set_req_nethash_create {
	uint32_t hashsize;
	uint16_t probes;
	uint16_t resize;
};

struct ip_set_req_nethash {
	ip_set_ip_t ip;
	unsigned char cidr;
};

static unsigned char shifts[] = {255, 253, 249, 241, 225, 193, 129, 1};

static inline ip_set_ip_t 
pack(ip_set_ip_t ip, unsigned char cidr)
{
	ip_set_ip_t addr, *paddr = &addr;
	unsigned char n, t, *a;

	addr = htonl(ip & (0xFFFFFFFF << (32 - (cidr))));
#ifdef __KERNEL__
	DP("ip:%u.%u.%u.%u/%u", NIPQUAD(addr), cidr);
#endif
	n = cidr / 8;
	t = cidr % 8;	
	a = &((unsigned char *)paddr)[n];
	*a = *a /(1 << (8 - t)) + shifts[t];
#ifdef __KERNEL__
	DP("n: %u, t: %u, a: %u", n, t, *a);
	DP("ip:%u.%u.%u.%u/%u, %u.%u.%u.%u",
	   HIPQUAD(ip), cidr, NIPQUAD(addr));
#endif

	return ntohl(addr);
}

#endif	/* __IP_SET_NETHASH_H */
