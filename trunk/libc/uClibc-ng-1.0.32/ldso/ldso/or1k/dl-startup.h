/* Startup code for the OpenRISC 1000 platform,
   based on microblaze implementation */
/*
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

__asm__ ("\
	.text\n\
	.globl _start\n\
	.type _start,@function\n\
	.hidden _start\n\
_start:\n\
	l.ori	r3, r9, 0\n\
	l.ori	r3, r1, 0\n\
	l.movhi	r11, 0\n\
1:\n\
	l.addi	r3, r3, 4\n\
	l.lwz	r12, 0(r3)\n\
	l.sfnei	r12, 0\n\
	l.addi	r11, r11, 1\n\
	l.bf	1b\n\
	 l.nop\n\
	l.ori	r3, r11, 0\n\
	l.ori	r3, r1, 0\n\
	l.addi	r11, r11, -1\n\
	/* store argument counter to stack */\n\
	l.sw	0(r3), r11\n\
	l.addi	r1, r1, -24\n\
	l.sw	0(r1), r9\n\
\n\
	l.jal	.LPC0\n\
#ifndef __OR1K_NODELAY__\n\
	 l.nop\n\
#endif\n\
	/* Load the PIC register */\n\
.LPC0:\n\
	l.movhi	r16, gotpchi(_GLOBAL_OFFSET_TABLE_+(.-.LPC0))\n\
	l.ori	r16, r16, gotpclo(_GLOBAL_OFFSET_TABLE_+(.-.LPC0))\n\
	l.add	r16, r16, r9\n\
\n\
	l.jal	_dl_start\n\
	 l.nop\n\
	/* FALLTHRU */\n\
\n\
	.globl _dl_start_user\n\
	.type _dl_start_user,@function\n\
_dl_start_user:\n\
	l.movhi	r12, gotoffhi(_dl_skip_args)\n\
	l.ori	r12, r12, gotofflo(_dl_skip_args)\n\
	l.add	r12, r12, r16\n\
	l.lwz	r12, 0(r12)\n\
	l.lwz	r3, 24(r1)\n\
\n\
	l.movhi	r9, gotoffhi(_dl_fini)\n\
	l.ori	r9, r9, gotofflo(_dl_fini)\n\
	l.add	r9, r9, r16\n\
\n\
	l.addi	r9, r9, -8\n\
	l.addi	r1, r1, 24\n\
	l.jr	r11\n\
	l.nop\n\
	.size _dl_start_user, . - _dl_start_user\n\
	.previous\n\
");
/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/* The ld.so library requires relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
			     unsigned long symbol_addr, unsigned long load_addr,
			     attribute_unused Elf32_Sym *symtab)
{

	switch (ELF_R_TYPE(rpnt->r_info))
	{
		case R_OR1K_RELATIVE:

			*reloc_addr = load_addr + rpnt->r_addend;
			break;

		default:
			_dl_exit(1);
			break;

	}

}
