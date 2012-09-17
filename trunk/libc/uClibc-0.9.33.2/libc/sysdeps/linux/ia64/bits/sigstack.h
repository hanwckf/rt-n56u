/* sigstack, sigaltstack definitions.
   Copyright (C) 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SIGNAL_H
# error "Never include this file directly.  Use <signal.h> instead"
#endif

#ifndef _SIGSTACK_H
#define _SIGSTACK_H	1

#if defined __UCLIBC_SUSV4_LEGACY__ || !defined __UCLIBC_STRICT_HEADERS__
/* Structure describing a signal stack (obsolete).  */
struct sigstack
  {
    __ptr_t ss_sp;		/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.  */
  };
#endif


/* Possible values for `ss_flags.'.  */
enum
{
  SS_ONSTACK = 1,
#define SS_ONSTACK	SS_ONSTACK
  SS_DISABLE
#define SS_DISABLE	SS_DISABLE
};

/* Minimum stack size for a signal handler.

   Yes, this should be 131072 but the constant got defined incorrectly
   in the kernel and we have to live with it.  Users should in any case
   use SIGSTKSZ as the size user-supplied buffers should have.  */
#define MINSIGSTKSZ	131027

/* System default stack size.  */
#define SIGSTKSZ	262144


/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    __ptr_t ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;

#endif	/* bits/sigstack.h */
