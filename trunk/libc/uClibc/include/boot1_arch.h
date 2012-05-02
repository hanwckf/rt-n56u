/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 */

asm("" \
"	.text\n"			\
"	.globl	_dl_boot\n"		\
"_dl_boot:\n"				\
"	.set noreorder\n"		\
"	bltzal $0, 0f\n"		\
"	nop\n"				\
"0:	.cpload $31\n"			\
"	.set reorder\n"			\
"	la $4, _DYNAMIC\n"		\
"	sw $4, -0x7ff0($28)\n"	        \
"	move $4, $29\n"			\
"	la $8, coff\n"			\
"	.set noreorder\n"		\
"	bltzal $0, coff\n"		\
"	nop\n"				\
"coff:	subu $8, $31, $8\n"		\
"	.set reorder\n"			\
"	la $25, _dl_boot2\n"	        \
"	addu $25, $8\n"			\
"	jalr $25\n"			\
"	lw $4, 0($29)\n"		\
"	la $5, 4($29)\n"		\
"	sll $6, $4, 2\n"		\
"	addu $6, $6, $5\n"		\
"	addu $6, $6, 4\n"		\
"	la $7, _dl_elf_main\n"		\
"	lw $25, 0($7)\n"		\
"	jr $25\n"			\
);

#define _dl_boot _dl_boot2
#define LD_BOOT(X)   static void __attribute__ ((unused)) _dl_boot (X)
