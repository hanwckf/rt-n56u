/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _KERNEL_TERMIOS_H
#define _KERNEL_TERMIOS_H 1

/* We need the definition of tcflag_t, cc_t, and speed_t.  */
#include <termios.h>

/* The following corresponds to the values from the Linux 2.1.20 kernel.  */




#if defined( __sparc__ )


#define __KERNEL_NCCS 17
struct __kernel_termios
{
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_line;		/* line discipline */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
};



#elif defined(__powerpc__)



#define __KERNEL_NCCS 19
struct __kernel_termios
  {
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
    cc_t c_line;		/* line discipline */
    speed_t c_ispeed;           /* input speed */
    speed_t c_ospeed;           /* output speed */
  };

#define _HAVE_C_ISPEED 1
#define _HAVE_C_OSPEED 1

/* We have the kernel termios structure, so we can presume this code knows
   what it's doing...  */
#undef  TCGETS
#undef  TCSETS
#undef  TCSETSW
#undef  TCSETSF
#define TCGETS	_IOR ('t', 19, struct __kernel_termios)
#define TCSETS	_IOW ('t', 20, struct __kernel_termios)
#define TCSETSW	_IOW ('t', 21, struct __kernel_termios)
#define TCSETSF	_IOW ('t', 22, struct __kernel_termios)



#elif defined(__mips__)



#define __KERNEL_NCCS 23
struct __kernel_termios
{
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_line;		/* line discipline */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
};



#elif defined(__alpha__)



#define __KERNEL_NCCS 19
struct __kernel_termios
{
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
    cc_t c_line;		/* line discipline */
    speed_t c_ispeed;		/* input speed */
    speed_t c_ospeed;		/* output speed */
};

#define _HAVE_C_ISPEED 1
#define _HAVE_C_OSPEED 1



#else /* None of the above */



#define __KERNEL_NCCS 19
struct __kernel_termios
{
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_line;		/* line discipline */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
};

#endif




#endif /* kernel_termios.h */

