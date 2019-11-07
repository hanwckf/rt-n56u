/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

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

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* Define a macro which expands inline into the wrapper code for a system
   call.  */
# undef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr, args...)                              \
  ({                                                                    \
    INTERNAL_SYSCALL_DECL (_sc_err);                                    \
    unsigned long _sc_val = INTERNAL_SYSCALL (name, _sc_err, nr, args); \
    if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_sc_val, _sc_err), 0)) \
    {                                                                   \
      __set_errno (INTERNAL_SYSCALL_ERRNO (_sc_val, _sc_err));          \
      _sc_val = -1;                                                     \
    }                                                                   \
    (long) _sc_val;                                                     \
  })

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)        \
  internal_syscall##nr (SYS_ify (name), err, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)  \
  internal_syscall##nr (number, err, args)

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) int err

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) ({ (void) (val); (err) != 0; })

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err) ({ (void) (val); (err); })

#define internal_syscall0(num, err, dummy...)                           \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num)                                                     \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall1(num, err, arg0)                               \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0)                                       \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall2(num, err, arg0, arg1)                         \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1)                         \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall3(num, err, arg0, arg1, arg2)                   \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2)           \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall4(num, err, arg0, arg1, arg2, arg3)             \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3)                                                    \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall5(num, err, arg0, arg1, arg2, arg3, arg4)       \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3), "R04" (arg4)                                      \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall6(num, err, arg0, arg1, arg2, arg3, arg4, arg5) \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3), "R04" (arg4), "R05" (arg5)                        \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#undef __SYSCALL_CLOBBERS
#define __SYSCALL_CLOBBERS                                      \
  "r6",  "r7",                                                  \
    "r8",  "r9",        "r11", "r12", "r13", "r14", "r15",      \
    "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",     \
    "r24", "r25", "r26", "r27", "r28", "r29", "memory"

/* gcc doesn't seem to allow an input operand to be clobbered, so we
   fake it with dummy outputs. */
#define __SYSCALL_CLOBBER_DECLS                                         \
  _clobber_r2, _clobber_r3, _clobber_r4, _clobber_r5, _clobber_r10

#define __SYSCALL_CLOBBER_OUTPUTS                                       \
  "=R02" (_clobber_r2), "=R03" (_clobber_r3), "=R04" (_clobber_r4),     \
    "=R05" (_clobber_r5), "=R10" (_clobber_r10)

#endif
