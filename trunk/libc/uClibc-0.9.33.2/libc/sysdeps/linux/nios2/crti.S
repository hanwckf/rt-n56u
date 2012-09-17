
   .section .init
   .balign 4
   .global _init
   .type   _init, @function
_init:
    addi sp, sp, -8
    stw  ra, 0(sp)
    stw  fp, 4(sp)

   .balign 4
   
   
   .section .fini
   .balign 4
   .global _fini
   .type   _fini, @function
_fini:
    addi sp, sp, -8
    stw  ra, 0(sp)
    stw  fp, 4(sp)
  .balign 4
