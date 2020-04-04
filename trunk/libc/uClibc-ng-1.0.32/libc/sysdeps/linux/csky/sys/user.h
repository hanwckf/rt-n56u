/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _SYS_USER_H
#define _SYS_USER_H  1

struct user_fpregs {
	unsigned long  fsr;         /* fpu status reg */
	unsigned long  fesr;        /* fpu exception status reg */
	unsigned long  fp[32];      /* fpu general regs */
};

struct user_regs {
#if defined(__CSKYABIV2__)
	unsigned long int uregs[34];  /* CSKY V2 has 32 general rgister */
#else
	unsigned long int uregs[18];  /* CSKY V1 has 16 general rgister */
#endif
};

/*
 * When the kernel dumps core, it starts by dumping the user struct -
 * this will be used by gdb to figure out where the data and stack segments
 * are within the file, and what virtual addresses to use.
 */
struct user{
/* We start with the registers, to mimic the way that "memory" is returned
   from the ptrace(3,...) function.  */
	struct user_regs    regs; /* Where the registers are actually stored */
	int                 u_fpvalid;  /* True if math co-processor being used. */

/* The rest of this junk is to help gdb figure out what goes where */
	unsigned long int   u_tsize;	/* Text segment size (pages). */
	unsigned long int   u_dsize;	/* Data segment size (pages). */
	unsigned long int   u_ssize;	/* Stack segment size (pages). */
	unsigned long       start_code; /* Starting virtual address of text. */
	unsigned long       start_stack;/* Starting virtual address of stack area.
					   This is actually the bottom of the stack,
					   the top of the stack is always found in
					   the esp register.  */
	long int            signal;     /* Signal that caused the core dump. */
	int                 reserved;	/* No longer used */
	struct user_regs *  u_ar0;	/* Used by gdb to help find the values
					   for the registers. */
	unsigned long       magic;	/* To uniquely identify a core file */
	char                u_comm[32];	/* User command that was responsible */
	struct user_fpregs  u_fp;
	struct user_fpregs* u_fpstate;	/* Math Co-processor pointer. */
};

#endif /* _SYS_USER_H */
