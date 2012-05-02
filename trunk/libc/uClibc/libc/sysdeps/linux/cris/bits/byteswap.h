#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

#define ___swab16(x) \
({ \
	unsigned short __x = (x); \
	((unsigned short)( \
		(((unsigned short)(__x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(__x) & (unsigned short)0xff00U) >> 8) )); \
})

#define ___swab32(x) \
({ \
	unsigned long __x = (x); \
	((unsigned long)( \
		(((unsigned long)(__x) & (unsigned long)0x000000ffUL) << 24) | \
		(((unsigned long)(__x) & (unsigned long)0x0000ff00UL) <<  8) | \
		(((unsigned long)(__x) & (unsigned long)0x00ff0000UL) >>  8) | \
		(((unsigned long)(__x) & (unsigned long)0xff000000UL) >> 24) )); \
})

/* these are CRIS specific */

static inline unsigned short __fswab16(unsigned short x)
{
	__asm__ ("swapb %0" : "=r" (x) : "0" (x));
	
	return(x);
}

static inline unsigned long __fswab32(unsigned long x)
{
	__asm__ ("swapwb %0" : "=r" (x) : "0" (x));
	
	return(x);
}

#  define __bswap_16(x) \
(__builtin_constant_p((unsigned short)(x)) ? \
 ___swab16((x)) : \
 __fswab16((x)))

#  define __bswap_32(x) \
(__builtin_constant_p((unsigned long)(x)) ? \
 ___swab32((x)) : \
 __fswab32((x)))

#if defined __GNUC__ && __GNUC__ >= 2
/* Swap bytes in 64 bit value.  */
# define __bswap_constant_64(x) \
     ((((x) & 0xff00000000000000ull) >> 56)                                   \
      | (((x) & 0x00ff000000000000ull) >> 40)                                 \
      | (((x) & 0x0000ff0000000000ull) >> 24)                                 \
      | (((x) & 0x000000ff00000000ull) >> 8)                                  \
      | (((x) & 0x00000000ff000000ull) << 8)                                  \
      | (((x) & 0x0000000000ff0000ull) << 24)                                 \
      | (((x) & 0x000000000000ff00ull) << 40)                                 \
      | (((x) & 0x00000000000000ffull) << 56))

# define __bswap_64(x) \
     (__extension__                                                           \
      ({ union { __extension__ unsigned long long int __ll;                   \
                 unsigned int __l[2]; } __w, __r;                             \
         if (__builtin_constant_p (x))                                        \
           __r.__ll = __bswap_constant_64 (x);                                \
         else                                                                 \
           {                                                                  \
             __w.__ll = (x);                                                  \
             __r.__l[0] = __bswap_32 (__w.__l[1]);                            \
             __r.__l[1] = __bswap_32 (__w.__l[0]);                            \
           }                                                                  \
         __r.__ll; }))
#endif

#endif /* _BITS_BYTESWAP_H */
