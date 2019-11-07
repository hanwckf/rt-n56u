#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

#include <libc-symbols.h>

.text
.global C_SYMBOL_NAME(setjmp)
.type C_SYMBOL_NAME(setjmp),@function
.align 4

C_SYMBOL_NAME(setjmp):
    mvi     r2, 1       /* save the signal mask */
    bi      C_SYMBOL_NAME(__sigsetjmp)

.text
.global C_SYMBOL_NAME(_setjmp)
.type C_SYMBOL_NAME(_setjmp),@function
.align 4

C_SYMBOL_NAME(_setjmp):
    mvi     r2, 0       /* don't save the signal mask */
    bi      C_SYMBOL_NAME(__sigsetjmp)

.text
.global C_SYMBOL_NAME(__sigsetjmp)
.type C_SYMBOL_NAME(__sigsetjmp),@function
.align 4

C_SYMBOL_NAME(__sigsetjmp):
    /* save registers relative to r1 (arg0) */
    sw      (r1+0), r11
    sw      (r1+4), r12
    sw      (r1+8), r13
    sw      (r1+12), r14
    sw      (r1+16), r15
    sw      (r1+20), r16
    sw      (r1+24), r17
    sw      (r1+28), r18
    sw      (r1+32), r19
    sw      (r1+36), r20
    sw      (r1+40), r21
    sw      (r1+44), r22
    sw      (r1+48), r23
    sw      (r1+52), r24
    sw      (r1+56), r25
    sw      (r1+60), gp
    sw      (r1+64), fp
    sw      (r1+68), sp
    sw      (r1+72), ra

    /* make a tail call to __sigjmp_save; it takes the same args */
    bi      C_SYMBOL_NAME(__sigjmp_save)
