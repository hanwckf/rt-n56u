#ifndef _IP_NAT_SIP_H
#define _IP_NAT_SIP_H
/* SIP extension for UDP NAT alteration. */

/* Protects sip part of conntracks */
DECLARE_LOCK_EXTERN(ip_sip_lock);

struct ip_nat_sip_info
{
	u_int32_t via_ip;	/* original, before masq */
	u_int16_t via_port;	/* original, before masq */
};

#endif /* _IP_NAT_SIP_H */
