#ifndef _SYS_USER_H
#define _SYS_USER_H

struct user_fpregs
{

};

struct user_regs
{
	unsigned long sr;
	unsigned long pc;
	unsigned long lr;
	unsigned long sp;
	unsigned long r12;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long r7;
	unsigned long r6;
	unsigned long r5;
	unsigned long r4;
	unsigned long r3;
	unsigned long r2;
	unsigned long r1;
	unsigned long r0;
	unsigned long r12_orig;
};

struct user
{
	struct user_regs	regs;		/* general registers */
	size_t			u_tsize;	/* text size (pages) */
	size_t			u_dsize;	/* data size (pages) */
	size_t			u_ssize;	/* stack size (pages) */
	unsigned long		start_code;	/* text starting address */
	unsigned long		start_data;	/* data starting address */
	unsigned long		start_stack;	/* stack starting address */
	long int		signal;		/* signal causing core dump */
	struct user_regs *	u_ar0;		/* help gdb find registers */
	unsigned long		magic;		/* identifies a core file */
	char			u_comm[32];	/* user command name */
};

#endif /* _SYS_USER_H */
