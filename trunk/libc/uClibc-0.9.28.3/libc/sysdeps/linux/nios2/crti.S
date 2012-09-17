   .file   "initfini.c"
#APP
  
   .section .init
#NO_APP
   .balign 4
   .global __init
   .type   __init, @function
__init:
    addi sp, sp, -8
    stw  ra, 0(sp)
    stw  fp, 4(sp)
#APP
  
   .balign 4
   
   
   .section .fini
#NO_APP
   .balign 4
   .global __fini
   .type   __fini, @function
__fini:
    addi sp, sp, -8
    stw  ra, 0(sp)
    stw  fp, 4(sp)
#APP
  .balign 4
   
   
  .ident  "GCC: (GNU) 3.3.2"
