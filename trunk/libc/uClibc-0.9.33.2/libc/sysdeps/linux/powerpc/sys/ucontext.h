/* Copyright (C) 1998, 1999, 2002, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>

#if __WORDSIZE == 32

/* Number of general registers.  */
# define NGREG	48

/* Container for all general registers.  */
typedef unsigned long gregset_t[NGREG];

/* Container for floating-point registers and status */
typedef struct _libc_fpstate
{
	double fpregs[32];
	double fpscr;
	unsigned int _pad[2];
} fpregset_t;

/* Container for Altivec/VMX registers and status.
   Needs to be aligned on a 16-byte boundary. */
typedef struct _libc_vrstate
{
	unsigned int vrregs[32][4];
	unsigned int vrsave;
	unsigned int _pad[2];
	unsigned int vscr;
} vrregset_t;

/* Context to describe whole processor state.  */
typedef struct
{
	gregset_t gregs;
	fpregset_t fpregs;
	vrregset_t vrregs __attribute__((__aligned__(16)));
} mcontext_t;

#else

/* For 64-bit kernels with Altivec support, a machine context is exactly
 * a sigcontext.  For older kernel (without Altivec) the sigcontext matches
 * the mcontext upto but not including the v_regs field.  For kernels that
 * don't AT_HWCAP or return AT_HWCAP without PPC_FEATURE_HAS_ALTIVEC the
 * v_regs field may not exit and should not be referenced.  The v_regd field
 * can be refernced safely only after verifying that PPC_FEATURE_HAS_ALTIVEC
 * is set in AT_HWCAP.  */

/* Number of general registers.  */
# define NGREG	48	/* includes r0-r31, nip, msr, lr, etc.   */
# define NFPREG	33	/* includes fp0-fp31 &fpscr.  */
# define NVRREG	34	/* includes v0-v31, vscr, & vrsave in split vectors */

typedef unsigned long gregset_t[NGREG];
typedef double fpregset_t[NFPREG];

/* Container for Altivec/VMX Vector Status and Control Register.  Only 32-bits
   but can only be copied to/from a 128-bit vector register.  So we allocated
   a whole quadword speedup save/restore.  */
typedef struct _libc_vscr
{
	unsigned int __pad[3];
	unsigned int vscr_word;
} vscr_t;

/* Container for Altivec/VMX registers and status.
   Must to be aligned on a 16-byte boundary. */
typedef struct _libc_vrstate
{
	unsigned int	vrregs[32][4];
	vscr_t		vscr;
	unsigned int	vrsave;
	unsigned int	__pad[3];
} vrregset_t  __attribute__((__aligned__(16)));

typedef struct {
	unsigned long	__unused[4];
	int		signal;
	int		__pad0;
	unsigned long	handler;
	unsigned long	oldmask;
	struct pt_regs	*regs;
	gregset_t	gp_regs;
	fpregset_t	fp_regs;
/*
 * To maintain compatibility with current implementations the sigcontext is
 * extended by appending a pointer (v_regs) to a quadword type (elf_vrreg_t)
 * followed by an unstructured (vmx_reserve) field of 69 doublewords.  This
 * allows the array of vector registers to be quadword aligned independent of
 * the alignment of the containing sigcontext or ucontext. It is the
 * responsibility of the code setting the sigcontext to set this pointer to
 * either NULL (if this processor does not support the VMX feature) or the
 * address of the first quadword within the allocated (vmx_reserve) area.
 *
 * The pointer (v_regs) of vector type (elf_vrreg_t) is essentually
 * an array of 34 quadword entries.  The entries with
 * indexes 0-31 contain the corresponding vector registers.  The entry with
 * index 32 contains the vscr as the last word (offset 12) within the
 * quadword.  This allows the vscr to be stored as either a quadword (since
 * it must be copied via a vector register to/from storage) or as a word.
 * The entry with index 33 contains the vrsave as the first word (offset 0)
 * within the quadword.
 */
	vrregset_t	*v_regs;
	long		vmx_reserve[NVRREG+NVRREG+1];
} mcontext_t;

#endif

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
#if __WORDSIZE == 32
    /*
     * These fields are set up this way to maximize source and
     * binary compatibility with code written for the old
     * ucontext_t definition, which didn't include space for the
     * registers.
     *
     * Different versions of the kernel have stored the registers on
     * signal delivery at different offsets from the ucontext struct.
     * Programs should thus use the uc_mcontext.uc_regs pointer to
     * find where the registers are actually stored.  The registers
     * will be stored within the ucontext_t struct but not necessarily
     * at a fixed address.  As a side-effect, this lets us achieve
     * 16-byte alignment for the register storage space if the
     * Altivec registers are to be saved, without requiring 16-byte
     * alignment on the whole ucontext_t.
     *
     * The uc_mcontext.regs field is included for source compatibility
     * with programs written against the older ucontext_t definition,
     * and its name should therefore not change.  The uc_pad field
     * is for binary compatibility with programs compiled against the
     * old ucontext_t; it ensures that uc_mcontext.regs and uc_sigmask
     * are at the same offset as previously.
     */
    int uc_pad[7];
    union uc_regs_ptr {
      struct pt_regs *regs;
      mcontext_t *uc_regs;
    } uc_mcontext;
    sigset_t    uc_sigmask;
    char uc_reg_space[sizeof(mcontext_t) + 12];  /* last for extensibility */
#else /* 64-bit */
    sigset_t    uc_sigmask;
    mcontext_t  uc_mcontext;  /* last for extensibility */
#endif
  } ucontext_t;

#endif /* sys/ucontext.h */
