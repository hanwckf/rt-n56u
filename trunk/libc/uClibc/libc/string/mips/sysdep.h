/* Adapted from glibc's sysdeps/unix/mips/sysdep.h */

/* Copyright (C) 1992, 1995, 1997, 1999, 2000, 2002, 2003
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

#ifdef __ASSEMBLER__

#include <sgidefs.h>
#include <sys/regdef.h>

#define ENTRY(name) \
  .globl name;                                                                \
  .align 2;                                                                   \
  .ent name,0;                                                                \
  name##:

#undef END
#define END(function)                                   \
                .end    function;                       \
                .size   function,.-function

#if _MIPS_SIM == _MIPS_SIM_ABI32 || _MIPS_SIM == _MIPS_SIM_ABIO64
# define L(label) $L ## label
#else
# define L(label) .L ## label
#endif

#ifdef libc_hidden_builtin_def
#error WHOA!!! libc_hidden_builtin_def is defined
#else
#define libc_hidden_builtin_def(name)
#endif

#endif
