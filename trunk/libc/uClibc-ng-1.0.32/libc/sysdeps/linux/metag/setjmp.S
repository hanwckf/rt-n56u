/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

!!! setjmp and variants
        .text

!! int _setjmp (jmp_buf __env)
!! Store the calling environment in ENV, not saving the signal mask.
!! Return 0.  */
        .global __setjmp
        .type   __setjmp,function
__setjmp:
        MOV     D0Ar2,#0
        B       ___sigsetjmp1
        .size   __setjmp,.-__setjmp

!! int setjmp (jmp_buf __env)
!! Store the calling environment in ENV, also saving the signal mask.
!! Return 0.  */
        .global _setjmp
        .type   _setjmp,function
_setjmp:
        MOV     D0Ar2,#1
        !! fall through to __sigsetjmp
        .size   _setjmp,.-_setjmp

!! int __sigsetjmp (jmp_buf __env, int __savemask)
!! Store the calling environment in ENV, also saving the
!! signal mask if SAVEMASK is nonzero.  Return 0.
!! This is the internal name for `sigsetjmp'.
        .global ___sigsetjmp
        .type   ___sigsetjmp,function
___sigsetjmp:
___sigsetjmp1:
        !! Save A0/A1 regs
        MSETL   [D1Ar1++],A0.0,A0.1
        !! Use A0.3 as temp
        MOV     A0.3,D1Ar1
        !! Rewind D1Ar1 that was modified above
        SUB     D1Ar1,D1Ar1,#(2*8)
        !! Save D0/D1 regs
        MSETL   [A0.3++],D0FrT,D0.5,D0.6,D0.7
        !! Tail call __sigjmp_save
#ifdef __PIC__
        B       ___sigjmp_save@PLT
#else
        B       ___sigjmp_save
#endif
        .size   ___sigsetjmp,.-___sigsetjmp
