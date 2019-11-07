.section .init
.global _init
.type _init,@function
.align 4

_init:
    addi    sp, sp, -4
    sw      (sp+4), ra

.section .fini
.global _fini
.type _fini,@function
.align 4

_fini:
    addi    sp, sp, -4
    sw      (sp+4), ra
