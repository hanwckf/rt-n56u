/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifdef __CSKYABIV2__

__asm__ (
    "    .text\n\t"
    "    .globl _start\n\t"
    "_start:\n\t"
    "    mov a0, sp\n\t"
    "    bsr _dl_start\n\t"
    "    # Return from _dl_start, user entry point address in a0 \n\t"
    "    # the code is PIC, so get global offset table\n\t"
    "    grs gb,.Lgetpc1\n\t"
    ".Lgetpc1:\n\t    "
    "    lrw t0, .Lgetpc1@GOTPC\n\t"
    "    add gb, gb,t0\n\t"
    "    lrw r5, _dl_skip_args@GOT\n\t"
    "    ldr.w r5, (gb, r5 << 0)\n\t"
    "    # get the value of variable _dl_skip_args in r6\n\t"
    "    ldw r6, (r5, 0)\n\t"
    "    # get the argc in r7 \n\t"
    "    ldw r7, (sp, 0)\n\t"
    "    # adjust the argc, this may be a bug when _dl_skip_args > argc\n\t"
    "    rsub r6, r7\n\t"
    "    # adjust the stack\n\t"
    "    mov r7, r6\n\t"
    "    lsli r6, 2\n\t"
    "    # adjust the stack pointer,this may be a bug, "
    "    # because it must be 8 bytes align"
    "    addu sp, r6\n\t"
    "    stw  r7, (sp, 0)\n\t"
    "    lrw  r7, _dl_fini@GOTOFF\n\t"
    "    addu  r7, gb\n\t"
    "    jmp a0"
);
#else
__asm__ (
    "    .text\n\t"
    "    .globl _start\n\t"
    "_start:\n\t"
    "    mov r2, r0\n\t"
# if defined(__ck810__)
    "    bsr _dl_start\n\t"
#else
    "    # the code is PIC, so get global offset table\n\t"
    "    bsr .Lgetpc0\n\t"
    ".Lgetpc0:\n\t    "
    "    lrw r14, .Lgetpc0@GOTPC\n\t"
    "    add r14, r15\n\t"
    "    lrw r4, _dl_start@GOTOFF\n\t"
    "    add r4, r14\n\t"
    "    jsr r4\n\t"
#endif
    "    # Return from _dl_start, user entry point address in r2 \n\t"
    "    # the code is PIC, so get global offset table\n\t"
    "    bsr .Lgetpc1\n\t"
    ".Lgetpc1:\n\t    "
    "    lrw r3, .Lgetpc1@GOTPC\n\t"
    "    add r3, r15\n\t"
# if defined(__ck810__)
    "    ldw r5, (r3, _dl_skip_args@GOT)\n\t"
#else
    "    lrw r4, _dl_skip_args@GOT\n\t"
    "    add r4, r3\n\t"
    "    ldw r5, (r4, 0)\n\t"
#endif
    "    # get the value of variable _dl_skip_args in r6\n\t"
    "    ldw r6, (r5, 0)\n\t"
    "    # get the argc in r7 \n\t"
    "    ldw r7, (r0, 0)\n\t"
    "    # adjust the argc, this may be a bug when _dl_skip_args > argc\n\t"
    "    rsub r6, r7\n\t"
    "    # adjust the stack\n\t"
    "    mov r7, r6\n\t"
    "    lsli r6, 2\n\t"
    "    # adjust the stack pointer,this may be a bug, "
    "    # because it must be 8 bytes align"
    "    addu r0, r6\n\t"
    "    stw  r7, (r0, 0)\n\t"
    "    lrw  r7, _dl_fini@GOTOFF\n\t"
    "    addu  r7, r3\n\t"
    "    jmp r2"
);

#endif

/* Get a pointer to the argv array. */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Function calls are not safe until the GOT relocations have been done.  */
#define NO_FUNCS_BEFORE_BOOTSTRAP
/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
    unsigned long symbol_addr, unsigned long load_addr, attribute_unused Elf32_Sym *symtab)
{
    switch (ELF32_R_TYPE(rpnt->r_info))
    {
       case R_CKCORE_RELATIVE:
            *reloc_addr = load_addr + rpnt->r_addend;
            break;
        case R_CKCORE_GLOB_DAT:
        case R_CKCORE_JUMP_SLOT:
            *reloc_addr = symbol_addr;
            break;
        case R_CKCORE_ADDR32:
            *reloc_addr = symbol_addr + rpnt->r_addend;
            break;
        case R_CKCORE_NONE:
            break;

        default:
            _dl_exit(1);
    }
}


