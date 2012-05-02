/* Startup code compliant to the ELF CRIS ABI */

/* The first piece of initialized data.  */
int __data_start = 0;

static void start1 (int argc, char **argv) __attribute__ ((used, noreturn));

/* 
 * It is important that this be the first function.
 * This file is the first thing in the text section.  
 * This is implemented completely in assembler to avoid that the
 * compiler pushes stuff on the stack (e.g. the frame pointer when
 * debuging).
 */

/*
 * On the stack we have argc. We can calculate argv/envp
 * from that and the succeeding stack location, but fix so
 * we get the right calling convention (regs in r10/r11).
 *
 * Please view linux/fs/binfmt_elf.c for a complete
 * understanding of this.
 */
__asm__ ( \
          ".text\n\t" \
          ".global _start\n\t" \
          "_start:\n\t" \
          "pop $r10\n\t" \
          "move.d $sp, $r11\n\t" \
          "jump start1\n\t");

#include <features.h>

extern void __uClibc_main(int argc, char **argv, char **envp)
         __attribute__ ((__noreturn__));
extern void __uClibc_start_main(int argc, char **argv, char **envp, 
	void (*app_init)(void), void (*app_fini)(void))
         __attribute__ ((__noreturn__));
extern void weak_function _init(void);
extern void weak_function _fini(void);

/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
extern void main(int argc,void *argv,void *envp);
void (*__mainp)(int argc,void *argv,void *envp) = main;

static void
start1 (int argc, char **argv)
{
	char** environ;

	/* The environment starts just after ARGV.  */
	environ = &argv[argc + 1];
	
	/* 
	 * If the first thing after ARGV is the arguments
	 * themselves, there is no environment.  
	 */
	if ((char *) environ == *argv)
		/* 
		 * The environment is empty.  Make environ
		 * point at ARGV[ARGC], which is NULL.  
		 */
		--environ;
	
#if defined L_crt0 || ! defined __UCLIBC_CTOR_DTOR__
	/* Leave control to the libc */
	__uClibc_main(argc, argv, environ);
#else
	__uClibc_start_main(argc, argv, environ, _init, _fini);
#endif
}
