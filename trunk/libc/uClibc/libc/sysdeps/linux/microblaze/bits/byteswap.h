/*
 * libc/sysdeps/linux/microblaze/bits/byteswap.h -- Macros to swap the order
 * 	of bytes in integer values
 *
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *  Copyright (C) 1997,1998,2001  Free Software Foundation, Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 */

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

/* Swap bytes in 16 bit value.  */
#define __bswap_constant_16(x) \
  ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

# define __bswap_16(x) __bswap_constant_16 (x)

/* Swap bytes in 32 bit value.  */
#define __bswap_constant_32(x) \
  ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
   (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

# define __bswap_32(x) __bswap_constant_32 (x)

#if defined __GNUC__ && __GNUC__ >= 2
/* Swap bytes in 64 bit value.  */
# define __bswap_64(x)							      \
     (__extension__							      \
      ({ union { unsigned long long int __ll;				      \
		 unsigned long int __l[2]; } __bswap_64_v, __bswap_64_r;      \
	 __bswap_64_v.__ll = (x);					      \
	 __bswap_64_r.__l[0] = __bswap_32 (__bswap_64_v.__l[1]);	      \
	 __bswap_64_r.__l[1] = __bswap_32 (__bswap_64_v.__l[0]);	      \
	 __bswap_64_r.__ll; }))
#endif
