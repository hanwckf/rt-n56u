/* Copyright (C) 1998, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
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

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

#include <bits/sigcontext.h>

/*
 * These are here mostly for backwards compatibility with older Unices.
 * IA-64 Linux does not distinguish between "struct sigcontext" and
 * "ucontext_t" as all the necessary info is inside the former.
 */

typedef struct sigcontext mcontext_t;

#if defined __cplusplus && __GNUC_PREREQ (3, 5)
# define _SC_GR0_OFFSET	\
	__builtin_offsetof (struct sigcontext, sc_gr[0])
#elif defined __GNUC__
# define _SC_GR0_OFFSET	\
	(((char *) &((struct sigcontext *) 0)->sc_gr[0]) - (char *) 0)
#else
# define _SC_GR0_OFFSET	0xc8	/* pray that this is correct... */
#endif

typedef struct ucontext
  {
    union
      {
	mcontext_t _mc;
	struct
	  {
	    unsigned long _pad[_SC_GR0_OFFSET/8];
	    struct ucontext *_link;	/* this should overlay sc_gr[0] */
	  }
	_uc;
      }
    _u;
  }
ucontext_t;

#define uc_mcontext	_u._mc
#define uc_sigmask	_u._mc.sc_mask
#define uc_stack	_u._mc.sc_stack
#define uc_link		_u._uc._link

#endif /* sys/ucontext.h */
