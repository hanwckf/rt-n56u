
   .section .init

    ldw ra, 0(sp)
    ldw fp, 4(sp)
    addi sp, sp, 8
    ret

   .section .fini

    ldw ra, 0(sp)
    ldw fp, 4(sp)
    addi sp, sp, 8
    ret
