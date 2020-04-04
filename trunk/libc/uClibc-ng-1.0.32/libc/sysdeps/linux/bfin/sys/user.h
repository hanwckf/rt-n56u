#ifndef _SYS_USER_H
#define _SYS_USER_H

struct user_bfinfp_struct {
};

/* This is the old layout of "struct pt_regs" as of Linux 1.x, and
   is still the layout used by user (the new pt_regs doesn't have
   all registers). */
struct user_regs_struct {
	long r0, r1, r2, r3, r4, r5, r6, r7;
	long p0, p1, p2, p3, p4, p5, usp, fp;
	long i0, i1, i2, i3;
	long l0, l1, l2, l3;
	long b0, b1, b2, b3;
	long m0, m1, m2, m3;
	long a0w, a1w;
	long a0x, a1x;
	unsigned long rets;
	unsigned long astat;
	unsigned long pc;
	unsigned long orig_p0;
};

/* When the kernel dumps core, it starts by dumping the user struct -
   this will be used by gdb to figure out where the data and stack segments
   are within the file, and what virtual addresses to use. */

struct user {
/* We start with the registers, to mimic the way that "memory" is returned
   from the ptrace(3,...) function.  */

	struct user_regs_struct regs;	/* Where the registers are actually stored */

/* The rest of this junk is to help gdb figure out what goes where */
	unsigned long int u_tsize;	/* Text segment size (pages). */
	unsigned long int u_dsize;	/* Data segment size (pages). */
	unsigned long int u_ssize;	/* Stack segment size (pages). */
	unsigned long start_code;	/* Starting virtual address of text. */
	unsigned long start_stack;	/* Starting virtual address of stack area.
					   This is actually the bottom of the stack,
					   the top of the stack is always found in the
					   esp register.  */
	long int signal;	/* Signal that caused the core dump. */
	int reserved;		/* No longer used */
	unsigned long u_ar0;
	/* Used by gdb to help find the values for */
	/* the registers. */
	unsigned long magic;	/* To uniquely identify a core file */
	char u_comm[32];	/* User command that was responsible */
};
#define NBPG PAGE_SIZE
#define UPAGES 1
#define HOST_TEXT_START_ADDR (u.start_code)
#define HOST_STACK_END_ADDR (u.start_stack + u.u_ssize * NBPG)

#endif
