
/* Copyright (C) 2002, David McCullough <davidm@snapgear.com> */
/* This file is released under the LGPL, any version you like */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef	_ASM

typedef struct
  {
    unsigned long __dregs[6]; /* save d2 - d7 */
    unsigned long __aregs[6]; /* save a2 - a7 */
	unsigned long __pc;       /* the return address */

#if defined __HAVE_68881__ || defined __HAVE_FPU__
    /* There are eight floating point registers which
       are saved in IEEE 96-bit extended format.  */
    char __fpregs[8 * (96 / 8)];
#endif

  } __jmp_buf[1];

#endif /* _ASM */

#define JB_REGS   0
#define JB_DREGS  0
#define JB_AREGS  24
#define JB_PC     48
#define JB_FPREGS 52

#if defined __HAVE_68881__ || defined __HAVE_FPU__
# define JB_SIZE 76
#else
# define JB_SIZE 52
#endif


/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__aregs[5])
