.text
.globl _start
.type _start,@function
.type main,@function
.type __uClibc_main,@function

_start:
    mvi     fp, 0              /* clear fp */
    mvhi    r1, hi(main)       /* main */
    ori     r1, r1, lo(main)
    lw      r2, (sp+0)         /* argc */
    addi    r3, sp, 4          /* argv */
    mvhi    r4, hi(_init)      /* app_init */
    ori     r4, r4, lo(_init)
    mvhi    r5, hi(_fini)      /* app_fini */
    ori     r5, r5, lo(_fini)
    mvi     r6, 0              /* rtld_fini */
	mv      r7, sp             /* stack_end */
    
    calli   __uClibc_main

