/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

.macro      GET_FRONT_BITS rx ry
#ifdef      __cskyLE__
    lsr     \rx, \ry
#else
    lsl     \rx, \ry
#endif
.endm

.macro      GET_AFTER_BITS rx ry
#ifdef      __cskyLE__
    lsl     \rx, \ry
#else
    lsr     \rx, \ry
#endif
.endm


#ifdef WANT_WIDE
# define Wmemcpy wmemcpy
#else
# define Wmemcpy memcpy
#endif

/* void *memcpy(void *dest, const void *src, size_t n); */

	.text
	.align 2
	.global Wmemcpy
	.type   Wmemcpy, @function
Wmemcpy:
    mov     r7, r2
    cmplti  r4, 4                                   /* If len less than 4 bytes */
    jbt     .L_copy_by_byte

    mov     r6, r2
    andi    r6, 3
    cmpnei  r6, 0
    jbt     .L_dest_not_aligned                     /* If dest is not 4 bytes aligned */
.L0:
    mov     r6, r3
    andi    r6, 3
    cmpnei  r6, 0
    jbt     .L_dest_aligned_but_src_not_aligned     /* If dest is aligned, but src is not aligned */

    cmplti  r4, 16                                  /* dest and src are all aligned */
    jbt     .L_aligned_and_len_less_16bytes         /* If len less than 16 bytes */

    subi    sp, 8
    stw     r8, (sp, 0)
    stw     r9, (sp, 4)
.L_aligned_and_len_larger_16bytes:                  /* src and dst are all aligned, and len > 16 bytes */
    ldw     r1, (r3, 0)
    ldw     r5, (r3, 4)
    ldw     r8, (r3, 8)
    ldw     r9, (r3, 12)
    stw     r1, (r7, 0)
    stw     r5, (r7, 4)
    stw     r8, (r7, 8)
    stw     r9, (r7, 12)
    subi    r4, 16
    addi    r3, 16
    addi    r7, 16
    cmplti  r4, 16
    jbf     .L_aligned_and_len_larger_16bytes
    ldw     r8, (sp, 0)
    ldw     r9, (sp, 4)
    addi    sp, 8

.L_aligned_and_len_less_16bytes:
    cmplti  r4, 4
    jbt     .L_copy_by_byte
    ldw     r1, (r3, 0)
    stw     r1, (r7, 0)
    subi    r4, 4
    addi    r3, 4
    addi    r7, 4
    jbr     .L_aligned_and_len_less_16bytes

.L_copy_by_byte:                                    /* len less than 4 bytes */
    cmpnei  r4, 0
    jbf     .L_return
    ldb     r1, (r3, 0)
    stb     r1, (r7, 0)
    subi    r4, 1
    addi    r3, 1
    addi    r7, 1
    jbr     .L_copy_by_byte

.L_return:
    rts

/* If dest is not aligned, we copy some bytes to make dest align.
   Then we should judge whether src is aligned. */

.L_dest_not_aligned:
    mov     r5, r3                                  /* consider overlapped case */
    rsub    r5, r5, r7
    abs     r5, r5
    cmplt   r5, r4
    jbt     .L_copy_by_byte

.L1:
    ldb     r1, (r3, 0)                             /* makes the dest align. */
    stb     r1, (r7, 0)
    addi    r6, 1
    subi    r4, 1
    addi    r3, 1
    addi    r7, 1
    cmpnei  r6, 4
    jbt     .L1
    cmplti  r4, 4
    jbt     .L_copy_by_byte
    jbf     .L0                                     /* judge whether the src is aligned. */

.L_dest_aligned_but_src_not_aligned:
    mov     r5, r3                                  /* consider overlapped case*/
    rsub    r5, r5, r7
    abs     r5, r5
    cmplt   r5, r4
    jbt     .L_copy_by_byte

    bclri   r3, 0
    bclri   r3, 1
    ldw     r1, (r3, 0)
    addi    r3, 4

    subi    sp, 16
    stw     r11, (sp,0)
    stw     r12, (sp,4)
    stw     r13, (sp,8)
    movi    r5, 8
    mult    r5, r6                                  /* r6 is used to store tne misaligned bits */
    mov     r12, r5
    rsubi   r5, 31
    addi    r5, 1
    mov     r13, r5

    cmplti  r4, 16
    jbt     .L_not_aligned_and_len_less_16bytes

    stw     r8, (sp, 12)
    subi    sp, 8
    stw     r9, (sp, 0)
    stw     r10, (sp, 4)
.L_not_aligned_and_len_larger_16bytes:
    ldw     r5, (r3, 0)
    ldw     r11, (r3, 4)
    ldw     r8, (r3, 8)
    ldw     r9, (r3, 12)

    GET_FRONT_BITS r1 r12                          /* little or big endian? */
    mov     r10, r5
    GET_AFTER_BITS r5 r13
    or      r5, r1

    GET_FRONT_BITS r10 r12
    mov     r1, r11
    GET_AFTER_BITS r11 r13
    or      r11, r10

    GET_FRONT_BITS r1 r12
    mov     r10, r8
    GET_AFTER_BITS r8 r13
    or      r8, r1

    GET_FRONT_BITS r10 r12
    mov     r1, r9
    GET_AFTER_BITS r9 r13
    or      r9, r10

    stw     r5, (r7, 0)
    stw     r11, (r7, 4)
    stw     r8, (r7, 8)
    stw     r9, (r7, 12)
    subi    r4, 16
    addi    r3, 16
    addi    r7, 16
    cmplti  r4, 16
    jbf     .L_not_aligned_and_len_larger_16bytes
    ldw     r9, (sp, 0)
    ldw     r10, (sp, 4)
    addi    sp, 8
    ldw     r8, (sp,12)

.L_not_aligned_and_len_less_16bytes:
    cmplti  r4, 4
    jbf     .L2
    rsubi   r6, 4                                   /* r6 is used to stored the misaligned bits */
    subu    r3, r6                                 /* initial the position */
    ldw     r11, (sp, 0)
    ldw     r12, (sp, 4)
    ldw     r13, (sp, 8)
    addi    sp, 16
    jbr     .L_copy_by_byte
.L2:
    ldw     r5, (r3, 0)
    GET_FRONT_BITS r1 r12
    mov     r11, r1
    mov     r1, r5
    GET_AFTER_BITS r5 r13
    or      r5, r11
    stw     r5, (r7, 0)
    subi    r4, 4
    addi    r3, 4
    addi    r7, 4
    jbr     .L_not_aligned_and_len_less_16bytes

.size   Wmemcpy, .-Wmemcpy

libc_hidden_def(Wmemcpy)
.weak Wmemcpy
