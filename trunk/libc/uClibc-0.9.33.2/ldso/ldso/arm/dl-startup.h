/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <bits/arm_asm.h>

#if !defined(__thumb__)
__asm__(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,%function\n"
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
#if defined(__USE_BX__)
	"	bx	r6\n"
#else
	"	mov	pc, r6\n"
#endif
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
#if defined(__USE_BX__)
	"	bx	r6\n"
#else
	"	mov	pc, r6\n"
#endif
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


/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
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
			*reloc_addr += load_addr;
			break;
		case R_ARM_COPY:
			break;
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}
