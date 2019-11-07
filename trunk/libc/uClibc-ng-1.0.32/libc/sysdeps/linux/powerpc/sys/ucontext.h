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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>

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

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
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
  } ucontext_t;

#endif /* sys/ucontext.h */
