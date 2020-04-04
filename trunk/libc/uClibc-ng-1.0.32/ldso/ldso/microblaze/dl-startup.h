/* Startup code for the microblaze platform, based on glibc 2.3.6, dl-machine.h */

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__asm__ ("\
	.text\n\
	.globl _start\n\
	.type _start,@function\n\
	.hidden _start\n\
_start:\n\
	addk  r5,r0,r1\n\
	addk  r3,r0,r0\n\
1:\n\
	addik r5,r5,4\n\
	lw    r4,r5,r0\n\
	bneid r4,1b\n\
	addik r3,r3,1\n\
	addik r3,r3,-1\n\
	addk  r5,r0,r1\n\
	sw    r3,r5,r0\n\
	addik r1,r1,-24\n\
	sw    r15,r1,r0\n\
	brlid r15,_dl_start\n\
	nop\n\
	/* FALLTHRU */\n\
\n\
	.globl _dl_start_user\n\
	.type _dl_start_user,@function\n\
_dl_start_user:\n\
	mfs   r20,rpc\n\
	addik r20,r20,_GLOBAL_OFFSET_TABLE_+8\n\
	lwi   r4,r20,_dl_skip_args@GOTOFF\n\
	lwi   r5,r1,24\n\
	rsubk r5,r4,r5\n\
	addk  r4,r4,r4\n\
	addk  r4,r4,r4\n\
	addk  r1,r1,r4\n\
	swi   r5,r1,24\n\
	swi   r3,r1,20\n\
	addk  r6,r5,r0\n\
	addk  r5,r5,r5\n\
	addk  r5,r5,r5\n\
	addik r7,r1,28\n\
	addk  r8,r7,r5\n\
	addik r8,r8,4\n\
	lwi   r5,r1,24\n\
	lwi   r3,r1,20\n\
	addk  r4,r5,r5\n\
	addk  r4,r4,r4\n\
	addik r6,r1,28\n\
	addk  r7,r6,r4\n\
	addik r7,r7,4\n\
	addik r15,r20,_dl_fini@GOTOFF\n\
	addik r15,r15,-8\n\
	brad  r3\n\
	addik r1,r1,24\n\
	nop\n\
	.size _dl_start_user, . - _dl_start_user\n\
	.previous");

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, attribute_unused Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info))
	{
		case R_MICROBLAZE_REL:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		case R_MICROBLAZE_NONE:
			break;
		default:
			_dl_exit(1);
			break;
	}
}
