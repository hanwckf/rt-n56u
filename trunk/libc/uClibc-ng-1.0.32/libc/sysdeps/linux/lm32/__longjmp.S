#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

#include <libc-symbols.h>

.text
.global C_SYMBOL_NAME(__longjmp)
.type C_SYMBOL_NAME(__longjmp),@function
.align 4

C_SYMBOL_NAME(__longjmp):
    /* load registers relative from r5 (arg0) */
    lw      r11, (r1+0)
    lw      r12, (r1+4)
    lw      r13, (r1+8)
    lw      r14, (r1+12)
    lw      r15, (r1+16)
    lw      r16, (r1+20)
    lw      r17, (r1+24)
    lw      r18, (r1+28)
    lw      r19, (r1+32)
    lw      r20, (r1+36)
    lw      r21, (r1+40)
    lw      r22, (r1+44)
    lw      r23, (r1+48)
    lw      r24, (r1+52)
    lw      r25, (r1+56)
    lw      gp, (r1+60)
    lw      fp, (r1+64)
    lw      sp, (r1+68)
    lw      ra, (r1+72)

    mv      r1, r2          /* copy val */
    bne     r1, r0, 1f
    mvi     r1, 1           /* val was zero, set it to 1 */
1:
    ret

libc_hidden_def(__longjmp)
