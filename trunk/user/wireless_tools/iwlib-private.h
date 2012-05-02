/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->09
 *
 * Private common header for the Wireless Extension library...
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2009 Jean Tourrilhes <jt@hpl.hp.com>
 */

#ifndef IWLIB_PRIVATE_H
#define IWLIB_PRIVATE_H

/*#include "CHANGELOG.h"*/

/***************************** INCLUDES *****************************/

#include "iwlib.h"		/* Public header */

/* Make gcc understand that when we say inline, we mean it.
 * I really hate when the compiler is trying to be more clever than me,
 * because in this case gcc is not able to figure out functions with a
 * single call site, so not only I have to tag those functions inline
 * by hand, but then it refuse to inline them properly.
 * Total saving for iwevent : 150B = 0.7%.
 * Fortunately, in gcc 3.4, they now automatically inline static functions
 * with a single call site. Hurrah !
 * Jean II */
#undef IW_GCC_HAS_BROKEN_INLINE
#if __GNUC__ == 3
#if __GNUC_MINOR__ >= 1 && __GNUC_MINOR__ < 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif	/* __GNUC_MINOR__ */
#endif	/* __GNUC__ */
/* However, gcc 4.0 has introduce a new "feature", when compiling with
 * '-Os', it does not want to inline iw_ether_cmp() and friends.
 * So, we need to fix inline again !
 * Jean II */
#if __GNUC__ == 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif	/* __GNUC__ */
/* Now, really fix the inline */
#ifdef IW_GCC_HAS_BROKEN_INLINE
#ifdef inline
#undef inline
#endif	/* inline */
#define inline		inline		__attribute__((always_inline))
#endif	/* IW_GCC_HAS_BROKEN_INLINE */

#ifdef __cplusplus
extern "C" {
#endif

/****************************** DEBUG ******************************/

//#define DEBUG 1

/************************ CONSTANTS & MACROS ************************/

/* Paths */
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

/* Some usefull constants */
#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
/* For doing log10/exp10 without libm */
#define LOG10_MAGIC	1.25892541179

/* Backward compatibility for network headers */
#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#endif /* ARPHRD_IEEE80211 */

#ifndef IW_EV_LCP_PK_LEN
/* Size of the Event prefix when packed in stream */
#define IW_EV_LCP_PK_LEN	(4)
/* Size of the various events when packed in stream */
#define IW_EV_CHAR_PK_LEN	(IW_EV_LCP_PK_LEN + IFNAMSIZ)
#define IW_EV_UINT_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(__u32))
#define IW_EV_FREQ_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_freq))
#define IW_EV_PARAM_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_param))
#define IW_EV_ADDR_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct sockaddr))
#define IW_EV_QUAL_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_quality))
#define IW_EV_POINT_PK_LEN	(IW_EV_LCP_PK_LEN + 4)
#endif	/* IW_EV_LCP_PK_LEN */

#ifndef IW_EV_LCP_PK2_LEN
struct iw_pk_event
{
	__u16		len;			/* Real lenght of this stuff */
	__u16		cmd;			/* Wireless IOCTL */
	union iwreq_data	u;		/* IOCTL fixed payload */
} __attribute__ ((packed));
struct	iw_pk_point
{
  void __user	*pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
} __attribute__ ((packed));

#define IW_EV_LCP_PK2_LEN	(sizeof(struct iw_pk_event) - sizeof(union iwreq_data))
#define IW_EV_POINT_PK2_LEN	(IW_EV_LCP_PK2_LEN + sizeof(struct iw_pk_point) - IW_EV_POINT_OFF)
#endif	/* IW_EV_LCP_PK2_LEN */

/************************* INLINE FUNTIONS *************************/
/*
 * Functions that are so simple that it's more efficient inlining them
 * Most inline are private because gcc is fussy about inline...
 */

/*------------------------------------------------------------------*/
/*
 * Display an Ethernet Socket Address in readable format.
 */
static inline char *
iw_saether_ntop(const struct sockaddr *sap, char* bufp)
{
  iw_ether_ntop((const struct ether_addr *) sap->sa_data, bufp);
  return bufp;
}
/*------------------------------------------------------------------*/
/*
 * Input an Ethernet Socket Address and convert to binary.
 */
static inline int
iw_saether_aton(const char *bufp, struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  return iw_ether_aton(bufp, (struct ether_addr *) sap->sa_data);
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet broadcast address
 */
static inline void
iw_broad_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0xFF, ETH_ALEN);
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet NULL address
 */
static inline void
iw_null_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0x00, ETH_ALEN);
}

/*------------------------------------------------------------------*/
/*
 * Compare two ethernet addresses
 */
static inline int
iw_ether_cmp(const struct ether_addr* eth1, const struct ether_addr* eth2)
{
  return memcmp(eth1, eth2, sizeof(*eth1));
}

#ifdef __cplusplus
}
#endif

#endif	/* IWLIB_PRIVATE_H */
