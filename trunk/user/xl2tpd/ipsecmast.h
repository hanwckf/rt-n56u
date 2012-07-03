#ifndef _IPSECMAST_H
# define _IPSECMAST_H

/* Can be overwritten in xl2tpd.conf [global] saref refinfo = 22 */
# ifndef IP_IPSEC_REFINFO
/*
 * 22 has been assigned to IP_NODEFRAG in 2.6.36+ so we moved from 22 to 30
 */
#  define IP_IPSEC_REFINFO 30
# endif

# ifndef IPSEC_SAREF_NULL
typedef uint32_t IPsecSAref_t;

#  define IPSEC_SAREF_NULL ((IPsecSAref_t)0)
# endif
#endif
