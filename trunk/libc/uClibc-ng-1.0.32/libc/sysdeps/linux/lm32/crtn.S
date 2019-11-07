.section .init

    lw      ra, (sp+4)
    addi    sp, sp, 4
    ret

.section .fini

    lw      ra, (sp+4)
    addi    sp, sp, 4
    ret
