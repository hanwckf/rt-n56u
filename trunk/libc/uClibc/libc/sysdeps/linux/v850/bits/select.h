/*
 * include/bits/select.h -- fd_set operations
 *
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *  Copyright (C) 1997, 1998 Free Software Foundation, Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 */

#ifndef _SYS_SELECT_H
# error "Never use <bits/select.h> directly; include <sys/select.h> instead."
#endif

#ifdef __GNUC__

/* We don't use `memset' because this would require a prototype and
   the array isn't too big.  */
#define __FD_ZERO(s)							      \
  do {									      \
    unsigned int __i;							      \
    fd_set *__arr = (s);						      \
    for (__i = 0; __i < sizeof (fd_set) / sizeof (__fd_mask); ++__i)	      \
      __FDS_BITS (__arr)[__i] = 0;					      \
  } while (0)

#define __FD_SET(fd, s)						       	      \
  do {									      \
	int __fd = (fd);						      \
	void *__addr = (void *)&__FDS_BITS (s);				      \
	__asm__ __volatile__ ("set1 %0, [%1]"				      \
			      : /*nothing*/				      \
			      : "r" (__fd & 0x7), "r" (__addr + (__fd >> 3)));\
  } while (0)

#define __FD_CLR(fd, s)							      \
  do {									      \
	int __fd = (fd);						      \
	void *__addr = (void *)&__FDS_BITS (s);				      \
	__asm__ __volatile__ ("clr1 %0, [%1]"				      \
			      : /*nothing*/				      \
			      : "r" (__fd & 0x7), "r" (__addr + (__fd >> 3)));\
  } while (0)

#define __FD_ISSET(fd, s)						      \
  ({									      \
	int __fd = (fd);						      \
	void *__addr = (void *)&__FDS_BITS (s);				      \
	int res;							      \
	__asm__ ("tst1 %1, [%2]; setf nz, %0"				      \
		 : "=r" (res)						      \
		 : "r" (__fd & 0x7), "r" (__addr + (__fd >> 3)));	      \
	res;								      \
  })

#else /* !__GNUC__ */

#define __FD_SET(d, s)     (__FDS_BITS (s)[__FDELT(d)] |= __FDMASK(d))
#define __FD_CLR(d, s)     (__FDS_BITS (s)[__FDELT(d)] &= ~__FDMASK(d))
#define __FD_ISSET(d, s)   ((__FDS_BITS (s)[__FDELT(d)] & __FDMASK(d)) != 0)

#endif /* __GNUC__ */
