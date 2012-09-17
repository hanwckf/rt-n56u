   .file   "initfini.c"
#APP
  
   .section .init
#NO_APP
   .balign  4
   .globl  _init
   .type   _init, @function
#NO_APP
    ldw ra, 0(sp)
    ldw fp, 4(sp)
    addi sp, sp, 8
    ret
#APP
    
   .section .fini
#NO_APP
   .balign  4
   .globl  _fini
   .type   _fini, @function
#NO_APP
    ldw ra, 0(sp)
    ldw fp, 4(sp)
    addi sp, sp, 8
    ret
#APP
    
   .ident  "GCC: (GNU) 3.3.2"
