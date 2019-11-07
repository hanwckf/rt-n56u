/* Copyright (C) 1992,1997-2003,2004,2005,2006 Free Software Foundation, Inc.
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

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

# include <errno.h>

# ifdef SHARED
#  define INLINE_VSYSCALL(name, nr, args...) \
  ({									      \
    __label__ out;							      \
    __label__ iserr;							      \
    INTERNAL_SYSCALL_DECL (sc_err);					      \
    long int sc_ret;							      \
									      \
    if (__vdso_##name != NULL)						      \
      {									      \
	sc_ret = INTERNAL_VSYSCALL_NCS (__vdso_##name, sc_err, nr, ##args);   \
	if (!INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
	  goto out;							      \
	if (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err) != ENOSYS)		      \
	  goto iserr;							      \
      }									      \
									      \
    sc_ret = INTERNAL_SYSCALL (name, sc_err, nr, ##args);		      \
    if (INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
      {									      \
      iserr:								      \
        __set_errno (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err));		      \
        sc_ret = -1L;							      \
      }									      \
  out:									      \
    sc_ret;								      \
  })
# else
#  define INLINE_VSYSCALL(name, nr, args...) \
  INLINE_SYSCALL (name, nr, ##args)
# endif

# ifdef SHARED
#  define INTERNAL_VSYSCALL(name, err, nr, args...) \
  ({									      \
    __label__ out;							      \
    long int v_ret;							      \
									      \
    if (__vdso_##name != NULL)						      \
      {									      \
	v_ret = INTERNAL_VSYSCALL_NCS (__vdso_##name, err, nr, ##args);	      \
	if (!INTERNAL_SYSCALL_ERROR_P (v_ret, err)			      \
	    || INTERNAL_SYSCALL_ERRNO (v_ret, err) != ENOSYS)		      \
	  goto out;							      \
      }									      \
    v_ret = INTERNAL_SYSCALL (name, err, nr, ##args);			      \
  out:									      \
    v_ret;								      \
  })
# else
#  define INTERNAL_VSYSCALL(name, err, nr, args...) \
  INTERNAL_SYSCALL (name, err, nr, ##args)
# endif

# define INTERNAL_VSYSCALL_NO_SYSCALL_FALLBACK(name, err, nr, args...)	      \
  ({									      \
    long int sc_ret = ENOSYS;						      \
									      \
    if (__vdso_##name != NULL)						      \
      sc_ret = INTERNAL_VSYSCALL_NCS (__vdso_##name, err, nr, ##args);	      \
    else								      \
      err = 1 << 28;							      \
    sc_ret;								      \
  })

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETRES_VSYSCALL	1
# define HAVE_CLOCK_GETTIME_VSYSCALL	1

/* Define a macro which expands inline into the wrapper code for a VDSO
   call. This use is for internal calls that do not need to handle errors
   normally. It will never touch errno.
   On powerpc a system call basically clobbers the same registers like a
   function call, with the exception of LR (which is needed for the
   "sc; bnslr+" sequence) and CR (where only CR0.SO is clobbered to signal
   an error return status).  */
# define INTERNAL_VSYSCALL_NCS(funcptr, err, nr, args...) \
  ({									      \
    register void *r0  __asm__ ("r0");					      \
    register long int r3  __asm__ ("r3");				      \
    register long int r4  __asm__ ("r4");				      \
    register long int r5  __asm__ ("r5");				      \
    register long int r6  __asm__ ("r6");				      \
    register long int r7  __asm__ ("r7");				      \
    register long int r8  __asm__ ("r8");				      \
    register long int r9  __asm__ ("r9");				      \
    register long int r10 __asm__ ("r10");				      \
    register long int r11 __asm__ ("r11");				      \
    register long int r12 __asm__ ("r12");				      \
    LOAD_ARGS_##nr (funcptr, args);					      \
    __asm__ __volatile__						      \
      ("mtctr %0\n\t"							      \
       "bctrl\n\t"							      \
       "mfcr %0"							      \
       : "=&r" (r0),							      \
	 "=&r" (r3), "=&r" (r4), "=&r" (r5),  "=&r" (r6),  "=&r" (r7),	      \
	 "=&r" (r8), "=&r" (r9), "=&r" (r10), "=&r" (r11), "=&r" (r12)	      \
       : ASM_INPUT_##nr							      \
       : "cr0", "ctr", "lr", "memory");					      \
    err = (long int) r0;						      \
    (int) r3;								      \
  })

/* Define a macro which expands inline into the wrapper code for a system
   call. This use is for internal calls that do not need to handle errors
   normally. It will never touch errno.
   On powerpc a system call basically clobbers the same registers like a
   function call, with the exception of LR (which is needed for the
   "sc; bnslr+" sequence) and CR (where only CR0.SO is clobbered to signal
   an error return status).  */

# undef INTERNAL_SYSCALL_DECL
# define INTERNAL_SYSCALL_DECL(err) long int err __attribute__((unused))

# define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			\
(__extension__ \
  ({									\
    register long int r0  __asm__ ("r0");				\
    register long int r3  __asm__ ("r3");				\
    register long int r4  __asm__ ("r4");				\
    register long int r5  __asm__ ("r5");				\
    register long int r6  __asm__ ("r6");				\
    register long int r7  __asm__ ("r7");				\
    register long int r8  __asm__ ("r8");				\
    register long int r9  __asm__ ("r9");				\
    register long int r10 __asm__ ("r10");				\
    register long int r11 __asm__ ("r11");				\
    register long int r12 __asm__ ("r12");				\
    LOAD_ARGS_##nr(name, args);						\
    __asm__ __volatile__						\
      ("sc   \n\t"							\
       "mfcr %0"							\
       : "=&r" (r0),							\
	 "=&r" (r3), "=&r" (r4), "=&r" (r5),  "=&r" (r6),  "=&r" (r7),	\
	 "=&r" (r8), "=&r" (r9), "=&r" (r10), "=&r" (r11), "=&r" (r12)	\
       : ASM_INPUT_##nr							\
       : "cr0", "ctr", "memory");					\
    err = r0;								\
    (int) r3;								\
  }) \
)
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((void) (val), unlikely ((err) & (1 << 28)))

# define INTERNAL_SYSCALL_ERRNO(val, err)     (val)

extern void __illegally_sized_syscall_arg1(void);
extern void __illegally_sized_syscall_arg2(void);
extern void __illegally_sized_syscall_arg3(void);
extern void __illegally_sized_syscall_arg4(void);
extern void __illegally_sized_syscall_arg5(void);
extern void __illegally_sized_syscall_arg6(void);

# define LOAD_ARGS_0(name, dummy) \
	r0 = name
# define LOAD_ARGS_1(name, __arg1) \
	long int arg1 = (long int) (__arg1); \
	LOAD_ARGS_0(name, 0); \
	if (__builtin_classify_type (__arg1) != 5 && sizeof (__arg1) > 4) \
	  __illegally_sized_syscall_arg1 (); \
	r3 = arg1
# define LOAD_ARGS_2(name, __arg1, __arg2) \
	long int arg2 = (long int) (__arg2); \
	LOAD_ARGS_1(name, __arg1); \
	if (__builtin_classify_type (__arg2) != 5 && sizeof (__arg2) > 4) \
	  __illegally_sized_syscall_arg2 (); \
	r4 = arg2
# define LOAD_ARGS_3(name, __arg1, __arg2, __arg3) \
	long int arg3 = (long int) (__arg3); \
	LOAD_ARGS_2(name, __arg1, __arg2); \
	if (__builtin_classify_type (__arg3) != 5 && sizeof (__arg3) > 4) \
	  __illegally_sized_syscall_arg3 (); \
	r5 = arg3
# define LOAD_ARGS_4(name, __arg1, __arg2, __arg3, __arg4) \
	long int arg4 = (long int) (__arg4); \
	LOAD_ARGS_3(name, __arg1, __arg2, __arg3); \
	if (__builtin_classify_type (__arg4) != 5 && sizeof (__arg4) > 4) \
	  __illegally_sized_syscall_arg4 (); \
	r6 = arg4
# define LOAD_ARGS_5(name, __arg1, __arg2, __arg3, __arg4, __arg5) \
	long int arg5 = (long int) (__arg5); \
	LOAD_ARGS_4(name, __arg1, __arg2, __arg3, __arg4); \
	if (__builtin_classify_type (__arg5) != 5 && sizeof (__arg5) > 4) \
	  __illegally_sized_syscall_arg5 (); \
	r7 = arg5
# define LOAD_ARGS_6(name, __arg1, __arg2, __arg3, __arg4, __arg5, __arg6) \
	long int arg6 = (long int) (__arg6); \
	LOAD_ARGS_5(name, __arg1, __arg2, __arg3, __arg4, __arg5); \
	if (__builtin_classify_type (__arg6) != 5 && sizeof (__arg6) > 4) \
	  __illegally_sized_syscall_arg6 (); \
	r8 = arg6

# define ASM_INPUT_0 "0" (r0)
# define ASM_INPUT_1 ASM_INPUT_0, "1" (r3)
# define ASM_INPUT_2 ASM_INPUT_1, "2" (r4)
# define ASM_INPUT_3 ASM_INPUT_2, "3" (r5)
# define ASM_INPUT_4 ASM_INPUT_3, "4" (r6)
# define ASM_INPUT_5 ASM_INPUT_4, "5" (r7)
# define ASM_INPUT_6 ASM_INPUT_5, "6" (r8)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
