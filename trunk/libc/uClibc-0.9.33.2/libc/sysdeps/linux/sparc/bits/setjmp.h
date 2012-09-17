/* Copyright (C) 1997,1999,2000,2003 Free Software Foundation, Inc.
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

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H  1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#include <bits/wordsize.h>

#if 0 /*__WORDSIZE == 64*/

#ifndef _ASM
typedef struct __sparc64_jmp_buf
  {
    struct __sparc64_jmp_buf	*uc_link;
    unsigned long		uc_flags;
    unsigned long		uc_sigmask;
    struct __sparc64_jmp_buf_mcontext
      {
	unsigned long		mc_gregs[19];
	unsigned long		mc_fp;
	unsigned long		mc_i7;
	struct __sparc64_jmp_buf_fpu
	  {
	    union
	      {
		unsigned int	sregs[32];
		unsigned long	dregs[32];
		long double	qregs[16];
	      }			mcfpu_fpregs;
	    unsigned long	mcfpu_fprs;
	    unsigned long	mcfpu_gsr;
	    void		*mcfpu_fq;
	    unsigned char	mcfpu_qcnt;
	    unsigned char	mcfpu_qentsz;
	    unsigned char	mcfpu_enab;
	  }			mc_fpregs;
      }				uc_mcontext;
  } __jmp_buf[1];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((unsigned long int) (address) < (jmpbuf)->uc_mcontext.mc_fp)

#else

#if defined __USE_MISC || defined _ASM
# define JB_SP  0
# define JB_FP  1
# define JB_PC  2
#endif

#ifndef _ASM
typedef int __jmp_buf[3];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((int) (address) < (jmpbuf)[JB_SP])

#endif

#endif  /* bits/setjmp.h */
