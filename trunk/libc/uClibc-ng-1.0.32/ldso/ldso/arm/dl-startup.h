/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <bits/arm_bx.h>

#if defined(__FDPIC__)
#if !defined(__thumb__) || defined(__thumb2__)
__asm__(
	"	.text\n"
	"	.globl  _start\n"
	"	.type   _start,%function\n"
#if defined(__thumb2__)
	"	.thumb_func\n"
#endif
	"	.align 2\n"
	"_start:\n"
	/* We compute the parameters for __self_reloc:
	   - r0 is a pointer to the loadmap (either from r8 or r7 if rtld is
	   lauched in standalone mode)
	   - r1 is a pointer to the start of .rofixup section
	   - r2 is a pointer to the last word of .rofixup section

	   __self_reloc will fix indirect addresses in .rofixup
	   section and will return the relocated GOT value.
	*/
#if defined(__thumb2__)
	"	sub	r4, pc, #4\n"
#else
	"	sub	r4, pc, #8\n"
#endif
	"	ldr	r1, .L__ROFIXUP_LIST__\n"
	"	add	r1, r1, r4\n"
	"	ldr	r2, .L__ROFIXUP_END__\n"
	"	add	r2, r2, r4\n"
	"	movs	r0, r8\n"
	"	it	eq\n"
	"	moveq	r0, r7\n"
	"	push	{r7, r8, r9, r10}\n"
	"	bl	__self_reloc;\n"
	"	pop	{r7, r8, r9, r10}\n"
	/* We compute the parameters for dl_start(). See DL_START()
	   macro below.  The address of the user entry point is
	   returned in dl_main_funcdesc (on stack).  */
	"	mov	r1, r7\n"
	"	mov	r2, r8\n"
	"	mov	r3, r9\n"
	"	mov	r4, sp\n"
	"	sub	r5, sp, #8\n"
	"	sub sp, sp, #16\n"
	"	str	r4, [sp, #4]\n"
	"	str	r5, [sp, #0]\n"
	"	mov	r9, r0\n"
	/* Save r9 into r4, to preserve the GOT pointer.  */
	"	mov	r4, r9\n"
	"	bl _dl_start;\n"
	/* Now compute parameters for entry point according to FDPIC ABI.  */
	"	ldr	r10, .L_dl_fini_gotofffuncdesc\n"
	/* Save GOT value from r4.  */
	"	add	r10, r10, r4\n"
	"	ldr	r5, [sp, #8]\n"
	"	ldr	r9, [sp, #12]\n"
	"	add sp, sp, #16\n"
	"	bx	r5\n"
	".loopforever:\n"
	"	b	.loopforever\n"
	".L__ROFIXUP_LIST__:\n"
	"	.word	__ROFIXUP_LIST__ - _start\n"
	".L__ROFIXUP_END__:\n"
	"	.word	__ROFIXUP_END__ - _start\n"
	".L_dl_fini_gotofffuncdesc:\n"
	"	.word	_dl_fini(GOTOFFFUNCDESC)\n"
	"	.size	_start,.-_start\n"
	"	.previous\n"
);
#else /* !defined(__thumb__) */
#error Thumb-1 is not supported
#endif /* !defined(__thumb__) */
#else /* defined(__FDPIC__) */
#if !defined(__thumb__)
__asm__(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,%function\n"
	"	.hidden	_start\n"
	"_start:\n"
	"	@ at start time, all the args are on the stack\n"
	"	mov	r0, sp\n"
	"	bl	_dl_start\n"
	"	@ returns user entry point in r0\n"
	"	mov	r6, r0\n"
	"	@ we are PIC code, so get global offset table\n"
	"	ldr	sl, .L_GET_GOT\n"
	"	add	sl, pc, sl\n"
	".L_GOT_GOT:\n"
	"	@ See if we were run as a command with the executable file\n"
	"	@ name as an extra leading argument.\n"
	"	ldr	r4, .L_SKIP_ARGS\n"
	"	ldr	r4, [sl, r4]\n"
	"	@ get the original arg count\n"
	"	ldr	r1, [sp]\n"
	"	@ subtract _dl_skip_args from it\n"
	"	sub	r1, r1, r4\n"
	"	@ adjust the stack pointer to skip them\n"
	"	add	sp, sp, r4, lsl #2\n"
	"	@ get the argv address\n"
	"	add	r2, sp, #4\n"
	"	@ store the new argc in the new stack location\n"
	"	str	r1, [sp]\n"
	"	@ compute envp\n"
	"	add	r3, r2, r1, lsl #2\n"
	"	add	r3, r3, #4\n"
	"\n\n"
	"	@ load the finalizer function\n"
	"	ldr	r0, .L_FINI_PROC\n"
	"	ldr	r0, [sl, r0]\n"
	"	@ jump to the user_s entry point\n"
	"	" __stringify(BX(r6)) "\n"
	".L_GET_GOT:\n"
	"	.word	_GLOBAL_OFFSET_TABLE_ - .L_GOT_GOT - 4\n"
	".L_SKIP_ARGS:\n"
	"	.word	_dl_skip_args(GOTOFF)\n"
	".L_FINI_PROC:\n"
	"	.word	_dl_fini(GOT)\n"
	"\n\n"
    "	.size	_start,.-_start\n"
	".previous\n"
);
#else
__asm__(
    "	.text\n"
    "	.arm\n"
    "	.globl	_start\n"
    "	.type	_start,%function\n"
	"_start:\n"
	"	@ dumb: can't persuade the linker to make the start address\n"
	"	@ odd, so use an arm function and change to thumb (_dl_start\n"
	"	@ is thumb)\n"
	"	adr	r0, __dl_thumb_start+1\n"
	"	bx	r0\n"
	"\n\n"
    "	.thumb\n"
    "	.globl	__dl_thumb_start\n"
    "	.thumb_func\n"
    "	.type	__dl_thumb_start,%function\n"
	"__dl_thumb_start:\n"
	"	@ at start time, all the args are on the stack\n"
	"	mov	r0, sp\n"
	"	bl	_dl_start\n"
	"	@ returns user entry point in r0\n"
	"	mov	r6, r0\n"
	"	@ we are PIC code, so get global offset table\n"
	"	ldr	r7, .L_GET_GOT\n"
	".L_GOT_GOT:\n"
	"	add	r7, pc\n"
	"	@ See if we were run as a command with the executable file\n"
	"	@ name as an extra leading argument.\n"
	"	ldr	r4, .L_SKIP_ARGS\n"
	"	ldr	r4, [r7, r4]\n"
	"	@ get the original arg count\n"
	"	ldr	r1, [sp]\n"
	"	@ subtract _dl_skip_args from it\n"
	"	sub	r1, r1, r4\n"
	"	@ adjust the stack pointer to skip them\n"
	"	lsl	r4, r4, #2\n"
	"	add	sp, r4\n"
	"	@ get the argv address\n"
	"	add	r2, sp, #4\n"
	"	@ store the new argc in the new stack location\n"
	"	str	r1, [sp]\n"
	"	@ compute envp\n"
	"	lsl	r3, r1, #2\n"
	"	add	r3, r3, r2\n"
	"	add	r3, #4\n"
	"\n\n"
	"	@ load the finalizer function\n"
	"	ldr	r0, .L_FINI_PROC\n"
	"	ldr	r0, [r7, r0]\n"
	"	@ jump to the user_s entry point\n"
	"	" __stringify(BX(r6)) "\n"
	"\n\n"
	".L_GET_GOT:\n"
	"	.word	_GLOBAL_OFFSET_TABLE_ - .L_GOT_GOT - 4\n"
	".L_SKIP_ARGS:\n"
	"	.word	_dl_skip_args(GOTOFF)\n"
	".L_FINI_PROC:\n"
	"	.word	_dl_fini(GOT)\n"
	"\n\n"
    "	.size	_start,.-_start\n"
	".previous\n"
);
#endif
#endif /* defined(__FDPIC__) */

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, DL_LOADADDR_TYPE load_addr, Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_ARM_NONE:
			break;
		case R_ARM_ABS32:
			*reloc_addr += symbol_addr;
			break;
		case R_ARM_PC24:
			{
				unsigned long addend;
				long newvalue, topbits;

				addend = *reloc_addr & 0x00ffffff;
				if (addend & 0x00800000) addend |= 0xff000000;

				newvalue = symbol_addr - (unsigned long)reloc_addr + (addend << 2);
				topbits = newvalue & 0xfe000000;
				if (topbits != 0xfe000000 && topbits != 0x00000000)
				{
#if 0
					/* Don't bother with this during ldso initilization... */
					newvalue = fix_bad_pc24(reloc_addr, symbol_addr)
						- (unsigned long)reloc_addr + (addend << 2);
					topbits = newvalue & 0xfe000000;
					if (unlikely(topbits != 0xfe000000 && topbits != 0x00000000))
					{
						SEND_STDERR("R_ARM_PC24 relocation out of range\n");
						_dl_exit(1);
					}
#else
					SEND_STDERR("R_ARM_PC24 relocation out of range\n");
					_dl_exit(1);
#endif
				}
				newvalue >>= 2;
				symbol_addr = (*reloc_addr & 0xff000000) | (newvalue & 0x00ffffff);
				*reloc_addr = symbol_addr;
				break;
			}
		case R_ARM_GLOB_DAT:
		case R_ARM_JUMP_SLOT:
			*reloc_addr = symbol_addr;
			break;
		case R_ARM_RELATIVE:
			*reloc_addr = DL_RELOC_ADDR(load_addr, *reloc_addr);
			break;
		case R_ARM_COPY:
			break;
#ifdef __FDPIC__
		case R_ARM_FUNCDESC_VALUE:
			{
				struct funcdesc_value *dst = (struct funcdesc_value *) reloc_addr;

				dst->entry_point += symbol_addr;
				dst->got_value = load_addr.got_value;
			}
			break;
#endif
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}

#ifdef __FDPIC__
#undef DL_START
#define DL_START(X)   \
static void  __attribute__ ((used)) \
_dl_start (Elf32_Addr dl_boot_got_pointer, \
          struct elf32_fdpic_loadmap *dl_boot_progmap, \
          struct elf32_fdpic_loadmap *dl_boot_ldsomap, \
          Elf32_Dyn *dl_boot_ldso_dyn_pointer, \
          struct funcdesc_value *dl_main_funcdesc, \
          X)

/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  We return the address of the function's entry point to
 * _dl_boot, see boot1_arch.h.
 */
#define START()	do {							\
  struct elf_resolve *exec_mod = _dl_loaded_modules;			\
  dl_main_funcdesc->entry_point = _dl_elf_main;				\
  while (exec_mod->libtype != elf_executable)				\
    exec_mod = exec_mod->next;						\
  dl_main_funcdesc->got_value = exec_mod->loadaddr.got_value;		\
  return;								\
} while (0)

/* We use __aeabi_idiv0 in _dl_find_hash, so we need to have the raise
   symbol.  */
int raise(int sig)
{
  _dl_exit(1);
}
#endif /* __FDPIC__ */
