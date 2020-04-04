/* Copyright (C) 2000-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <common/sysdep.h>

#ifdef  __ASSEMBLER__

# define ALIGNARG(log2) log2
# define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* Define an entry point visible from C.  */
# define ENTRY(name)                          \
  .globl C_SYMBOL_NAME(name);                 \
  .type C_SYMBOL_NAME(name),@function;        \
  .align ALIGNARG(2);                         \
  C_LABEL(name)

# undef END
# define END(name) ASM_SIZE_DIRECTIVE(name)

/* Local label name for asm code.  */
# ifndef L
#  define L(name) $L##name
# endif

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
# ifdef __PIC__
#  define SYSCALL_ERROR_LABEL 0f
# else
#  define SYSCALL_ERROR_LABEL __syscall_error
# endif

# define DO_CALL(syscall_name, args)                \
    addik r12,r0,SYS_ify (syscall_name);            \
    brki  r14,8;                                    \
    addk  r0,r0,r0;

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)           \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);                   \
    addik r12,r0,-4095;                             \
    cmpu  r12,r12,r3;                               \
    bgei  r12,SYSCALL_ERROR_LABEL;

# undef PSEUDO_END
# define PSEUDO_END(name)                           \
  SYSCALL_ERROR_HANDLER;                            \
  END (name)

# undef PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)   \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);

# undef PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)                   \
  END (name)

/* The function has to return the error code.  */
# undef  PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args)    \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);                   \

# undef  PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name)                    \
  END (name)

# undef ret_NOERRNO
# define ret_NOERRNO                                \
  rtsd r15,8; addk r0,r0,r0;

# undef ret_ERRVAL
# define ret_ERRVAL                                 \
  rtsd r15,8; rsubk r3,r3,r0;

#ifdef __PIC__
# define SYSCALL_ERROR_LABEL_DCL 0
# if defined _LIBC_REENTRANT
#  define SYSCALL_ERROR_HANDLER                     \
SYSCALL_ERROR_LABEL_DCL:                            \
    addik r1,r1,-16;                                \
    swi   r15,r1,0;                                 \
    swi   r20,r1,8;                                 \
    rsubk r3,r3,r0;                                 \
    swi   r3,r1,12;                                 \
    mfs   r20,rpc;                                  \
    addik r20,r20,_GLOBAL_OFFSET_TABLE_+8;          \
    brlid r15,__errno_location@PLT;                 \
    nop;                                            \
    lwi   r4,r1,12;                                 \
    swi   r4,r3,0;                                  \
    lwi   r20,r1,8;                                 \
    lwi   r15,r1,0;                                 \
    addik r1,r1,16;                                 \
    rtsd  r15,8;                                    \
    addik r3,r0,-1;
# else /* !_LIBC_REENTRANT.  */
#  define SYSCALL_ERROR_HANDLER                     \
SYSCALL_ERROR_LABEL_DCL:                            \
    mfs   r12,rpc;                                  \
    addik r12,r12,_GLOBAL_OFFSET_TABLE_+8;          \
    lwi   r12,r12,errno@GOT;                        \
    rsubk r3,r3,r0;                                 \
    swi   r3,r12,0;                                 \
    rtsd  r15,8;                                    \
    addik r3,r0,-1;
# endif /* _LIBC_REENTRANT.  */
#else
#  define SYSCALL_ERROR_HANDLER  /* Nothing here; code in sysdep.S is used.  */
#endif /* PIC.  */

#endif
