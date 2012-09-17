#ifndef _BSWAP_H
#define	_BSWAP_H 1

#if !defined(__BYTE_ORDER) && defined(BYTE_ORDER)
# define __BYTE_ORDER BYTE_ORDER
#endif

#ifndef __BYTE_ORDER
#ifdef __linux__
#include <endian.h>
#else
#define	__LITTLE_ENDIAN	1234	/* least-significant byte first (vax, pc) */
#define	__BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define	__PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#if defined(sun386) || defined(i386)
#define	__BYTE_ORDER	__LITTLE_ENDIAN
#endif

#if defined(sparc)
#define	__BYTE_ORDER	__BIG_ENDIAN
#endif

#endif /* __linux__ */
#endif /* __BYTE_ORDER */


#ifndef __BYTE_ORDER
# error "Undefined __BYTE_ORDER"
#endif

#ifdef __linux__
#include <byteswap.h>
#else
#include <string.h>
static __inline__ uint32_t bswap_32(uint32_t x)
     {
       uint32_t res;

       swab((void*)&x, (void*)&res, sizeof(uint32_t));

       return res;
     }

static __inline__ uint16_t bswap_16(uint16_t x)
     {
       uint16_t res;

       swab((void*)&x, (void*)&res, sizeof(uint16_t));
       return res;
     }
#endif

#endif
