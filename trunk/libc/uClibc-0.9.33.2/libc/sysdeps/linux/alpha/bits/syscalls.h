/* Copyright (C) 1992, 1995, 1996, 2000, 2003, 2004, 2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#define INLINE_SYSCALL_NCS(name, nr, args...)	\
(__extension__					\
 ({						\
	long _sc_ret, _sc_err;			\
	inline_syscall##nr(name, args);		\
	if (unlikely (_sc_err))			\
	  {					\
	    __set_errno (_sc_ret);		\
	    _sc_ret = -1L;			\
	  }					\
	_sc_ret;				\
  })						\
)

#define INTERNAL_SYSCALL_NCS(name, err_out, nr, args...) \
(__extension__ \
 ({							\
	long _sc_ret, _sc_err;				\
	inline_syscall##nr(name, args);			\
	err_out = _sc_err;				\
	_sc_ret;					\
  }) \
)
#define INTERNAL_SYSCALL_DECL(err)		long int err
#define INTERNAL_SYSCALL_ERROR_P(val, err)	err
#define INTERNAL_SYSCALL_ERRNO(val, err)	val

#define inline_syscall_clobbers				\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"

/* If TLS is in use, we have a conflict between the PAL_rduniq primitive,
   as modeled within GCC, and explicit use of the R0 register.  If we use
   the register via the asm, the scheduler may place the PAL_rduniq insn
   before we've copied the data from R0 into _sc_ret.  If this happens
   we'll get a reload abort, since R0 is live at the same time it is
   needed for the PAL_rduniq.

   Solve this by using the "v" constraint instead of an asm for the syscall
   output.  We don't do this unconditionally to allow compilation with
   older compilers.  */

#ifdef HAVE___THREAD
#define inline_syscall_r0_asm
#define inline_syscall_r0_out_constraint	"=v"
#else
#define inline_syscall_r0_asm			__asm__("$0")
#define inline_syscall_r0_out_constraint	"=r"
#endif

/* It is moderately important optimization-wise to limit the lifetime
   of the hard-register variables as much as possible.  Thus we copy
   in/out as close to the asm as possible.  */

#define inline_syscall0(name, args...)				\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_0 = name;						\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2"				\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19)					\
	   : "0"(_sc_0)						\
	   : inline_syscall_clobbers,				\
	     "$16", "$17", "$18", "$20", "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall1(name,arg1)				\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_19 __asm__("$19");			\
	register long _tmp_16 = (long) (arg1);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3"				\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16)				\
	   : "0"(_sc_0), "2"(_sc_16)				\
	   : inline_syscall_clobbers,				\
	     "$17", "$18", "$20", "$21");			\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall2(name,arg1,arg2)				\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_19 __asm__("$19");			\
	register long _tmp_16 = (long) (arg1);			\
	register long _tmp_17 = (long) (arg2);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	_sc_17 = _tmp_17;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4"			\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17)		\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17)		\
	   : inline_syscall_clobbers,				\
	     "$18", "$20", "$21");				\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall3(name,arg1,arg2,arg3)			\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _tmp_16 = (long) (arg1);			\
	register long _tmp_17 = (long) (arg2);			\
	register long _tmp_18 = (long) (arg3);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	_sc_17 = _tmp_17;					\
	_sc_18 = _tmp_18;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4 %5"			\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17),		\
	     "=r"(_sc_18)					\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17),		\
	     "4"(_sc_18)					\
	   : inline_syscall_clobbers, "$20", "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall4(name,arg1,arg2,arg3,arg4)		\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _tmp_16 = (long) (arg1);			\
	register long _tmp_17 = (long) (arg2);			\
	register long _tmp_18 = (long) (arg3);			\
	register long _tmp_19 = (long) (arg4);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	_sc_17 = _tmp_17;					\
	_sc_18 = _tmp_18;					\
	_sc_19 = _tmp_19;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4 %5 %6"			\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17),		\
	     "=r"(_sc_18)					\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17),		\
	     "4"(_sc_18), "1"(_sc_19)				\
	   : inline_syscall_clobbers, "$20", "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall5(name,arg1,arg2,arg3,arg4,arg5)		\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _sc_20 __asm__("$20");			\
	register long _tmp_16 = (long) (arg1);			\
	register long _tmp_17 = (long) (arg2);			\
	register long _tmp_18 = (long) (arg3);			\
	register long _tmp_19 = (long) (arg4);			\
	register long _tmp_20 = (long) (arg5);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	_sc_17 = _tmp_17;					\
	_sc_18 = _tmp_18;					\
	_sc_19 = _tmp_19;					\
	_sc_20 = _tmp_20;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7"		\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17),		\
	     "=r"(_sc_18), "=r"(_sc_20)				\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17),		\
	     "4"(_sc_18), "1"(_sc_19), "5"(_sc_20)		\
	   : inline_syscall_clobbers, "$21");			\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall6(name,arg1,arg2,arg3,arg4,arg5,arg6)	\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _sc_20 __asm__("$20");			\
	register long _sc_21 __asm__("$21");			\
	register long _tmp_16 = (long) (arg1);			\
	register long _tmp_17 = (long) (arg2);			\
	register long _tmp_18 = (long) (arg3);			\
	register long _tmp_19 = (long) (arg4);			\
	register long _tmp_20 = (long) (arg5);			\
	register long _tmp_21 = (long) (arg6);			\
								\
	_sc_0 = name;						\
	_sc_16 = _tmp_16;					\
	_sc_17 = _tmp_17;					\
	_sc_18 = _tmp_18;					\
	_sc_19 = _tmp_19;					\
	_sc_20 = _tmp_20;					\
	_sc_21 = _tmp_21;					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7 %8"		\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17),		\
	     "=r"(_sc_18), "=r"(_sc_20), "=r"(_sc_21)		\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17), "4"(_sc_18),	\
	     "1"(_sc_19), "5"(_sc_20), "6"(_sc_21)		\
	   : inline_syscall_clobbers);				\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
