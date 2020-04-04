#define LDSO_NEED_DPNT

unsigned int _dl_nios2_get_gp_value(ElfW(Dyn) *dpnt);

unsigned int
_dl_nios2_get_gp_value (ElfW(Dyn) *dpnt)
{ 
	while (dpnt->d_tag != DT_NULL) {
		if (dpnt->d_tag == DT_NIOS2_GP) {
			return (unsigned int)(dpnt->d_un.d_ptr);
		}
		++dpnt;
	}
	return 0;
}

__asm__ (
".text\n"
".globl _start\n"
".type _start, %function\n"
"_start:\n"
"        /* At start time, all the args are on the stack.  */\n"
"        mov r4, sp\n"
"\n"
"        /* Start the calculation of the GOT pointer.  */\n"
"        nextpc r22\n"
"1:      movhi r8, %hiadj(_gp_got - 1b)\n"
"        addi r8, r8, %lo(_gp_got - 1b)\n"
"\n"
"        /* Figure out where _dl_start will need to return to.  */\n"
"        movhi ra, %hiadj(2f - 1b)\n"
"        addi ra, ra, %lo(2f - 1b)\n"
"        add ra, ra, r22\n"
"\n"
"        /* Finish the calculation of the GOT pointer.  */\n"
"        add r22, r22, r8\n"
"\n"
"        br _dl_start\n"
"\n"
"        /* Save the returned user entry point.  */\n"
"2:      mov r16, r2\n"
"\n"
"        /* Initialize gp.  */\n"
"        ldw r4, %got(_dl_saved_dpnt)(r22)\n"
"        ldw r8, %call(_dl_nios2_get_gp_value)(r22)\n"
"        callr r8\n"
"        mov gp, r2\n"
"\n"
"        /* Find the number of arguments to skip.  */\n"
"        ldw r8, %got(_dl_skip_args)(r22)\n"
"        ldw r8, 0(r8)\n"
"\n"
"        /* Find argc.  */\n"
"        ldw r5, 0(sp)\n"
"        sub r5, r5, r8\n"
"        stw r5, 0(sp)\n"
"\n"
"        /* Find the first unskipped argument.  */\n"
"        slli r8, r8, 2\n"
"        addi r6, sp, 4\n"
"        add r9, r6, r8\n"
"        mov r10, r6\n"
"\n"
"        /* Shuffle envp down.  */\n"
"        mov r7, r10\n"
"3:      ldw r11, 0(r9)\n"
"        stw r11, 0(r10)\n"
"        addi r9, r9, 4\n"
"        addi r10, r10, 4\n"
"        bne r11, zero, 3b\n"
"\n"
"        /* Shuffle auxv down.  */\n"
"4:      ldw r11, 4(r9)\n"
"        stw r11, 4(r10)\n"
"        ldw r11, 0(r9)\n"
"        stw r11, 0(r10)\n"
"        addi r9, r9, 8\n"
"        addi r10, r10, 8\n"
"        bne r11, zero, 4b\n"
"\n"
"        /* Jump to the user's entry point.  */\n"
"        jmp r16\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP

/* The ld.so library requires relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, attribute_unused Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_NIOS2_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		default:
			_dl_exit(1);
			break;
	}
}
