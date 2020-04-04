/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

/*
 * This function is not called directly. It is jumped when attempting to use a
 * symbol that has not yet been resolved.
 *
 *.plt*:
 *   subi r0, 32
 *   stw r2, (r0, 0)
 *   stw r3, (r0, 4)
 *   lrw r3, #offset
 *   ldw r2, (gb, 8)
 *   jmp r2
 */

.import _dl_linux_resolver

.text
.globl _dl_linux_resolve
.type  _dl_linux_resolve,@function

_dl_linux_resolve:
#ifdef __CSKYABIV1__
    stw  r4, (r0, 8)
    stw  r5, (r0, 12)
    stw  r6, (r0, 16)
    stw  r7, (r0, 20)
    stw  r15,(r0, 24)
    # load the ID of this module
    ldw  r2, (gb, 4)
    # r2 = id, r3 = offset(do it in plt*)
#ifdef __PIC__
    # get global offset table address_
    bsr .L2
.L2:
    lrw r7, .L2@GOTPC
    add r7, r15
    # get the address of function (_dl_linux_resolver) in got table
    lrw r6, _dl_linux_resolver@GOT
    add r6, r7
    ldw r5, (r6, 0)
    jsr r5
#else  /* no __PIC__ */
    jsri  _dl_linux_resolver      /* need modify */
#endif
    # Return from _dl_linux_resolver, the address of function is in r2
    mov  r1, r2
    # Restore the registers
    ldw  r2, (r0, 0)
    ldw  r3, (r0, 4)
    ldw  r4, (r0, 8)
    ldw  r5, (r0, 12)
    ldw  r6, (r0, 16)
    ldw  r7, (r0, 20)
    ldw  r15,(r0, 24)
    # Restore the r0, because r0 is subtracted in PLT table
    addi r0, 32
    # The address of function is in r1, call the function without saving pc
    jmp  r1
#else       /* __CSKYABIV1__ */
    subi sp, 20
    stm  a0-a3, (sp)
    stw  lr, (sp, 16)
    # a0 = id, a1 = offset(do it in plt*)
    ldw  a0,  (gb, 4)
    mov  a1, t1
    bsr  _dl_linux_resolver
    mov  t0, a0
    ldw  lr, (sp, 16)
    ldm  a0-a3, (sp)
    addi sp, 20
    jmp  t0

#endif
