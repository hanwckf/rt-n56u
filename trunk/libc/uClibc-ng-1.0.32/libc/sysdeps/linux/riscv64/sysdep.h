/* Assembly macros for RISC-V.
   Copyright (C) 2011-2018
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_RISCV_SYSDEP_H
#define _LINUX_RISCV_SYSDEP_H 1

#include <common/sysdep.h>

#ifdef __ASSEMBLER__

# include <sys/asm.h>

# define ENTRY(name) LEAF(name)

# define L(label) .L ## label

/* Performs a system call, handling errors by setting errno.  Linux indicates
   errors by setting a0 to a value between -1 and -4095.  */
# undef PSEUDO
# define PSEUDO(name, syscall_name, args)			\
  .text;							\
  .align 2;							\
  ENTRY (name);							\
  li a7, SYS_ify (syscall_name);				\
  scall;							\
  li a7, -4096;							\
  bgtu a0, a7, .Lsyscall_error ## name;

# undef PSEUDO_END
# define PSEUDO_END(sym) 					\
  SYSCALL_ERROR_HANDLER (sym)					\
  ret;								\
  END (sym)

# if !IS_IN_libc
#  if defined (__PIC__)
#   define SYSCALL_ERROR_HANDLER(name)				\
.Lsyscall_error ## name:					\
        la.tls.ie t1, errno;					\
	add t1, t1, tp;						\
	neg a0, a0;						\
	sw a0, 0(t1);						\
        li a0, -1;
#  else
#   define SYSCALL_ERROR_HANDLER(name)				\
.Lsyscall_error ## name:					\
        lui t1, %tprel_hi(errno);				\
        add t1, t1, tp, %tprel_add(errno);			\
	neg a0, a0;						\
        sw a0, %tprel_lo(errno)(t1);				\
        li a0, -1;
#  endif
# else
#  define SYSCALL_ERROR_HANDLER(name)				\
.Lsyscall_error ## name:					\
        j       __syscall_error;
# endif

/* Performs a system call, not setting errno.  */
# undef PSEUDO_NEORRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)	\
  .align 2;						\
  ENTRY (name);						\
  li a7, SYS_ify (syscall_name);			\
  scall;

# undef PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)			\
  END (name)

# undef ret_NOERRNO
# define ret_NOERRNO ret

/* Perfroms a system call, returning the error code.  */
# undef PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args) 	\
  PSEUDO_NOERRNO (name, syscall_name, args)		\
  neg a0, a0;

# undef PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name)			\
  END (name)

# undef ret_ERRVAL
# define ret_ERRVAL ret

#endif /* __ASSEMBLER__ */

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
# include <errno.h>
#endif

#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

#ifndef __ASSEMBLER__

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETRES_VSYSCALL	1
# define HAVE_CLOCK_GETTIME_VSYSCALL	1
# define HAVE_GETTIMEOFDAY_VSYSCALL	1
# define HAVE_GETCPU_VSYSCALL		1


extern long int __syscall_error (long int neg_errno);

#endif /* ! __ASSEMBLER__ */

#endif /* linux/riscv/sysdep.h */
