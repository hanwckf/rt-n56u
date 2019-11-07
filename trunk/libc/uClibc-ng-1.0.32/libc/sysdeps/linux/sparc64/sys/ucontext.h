/* Copyright (C) 1998-2017 Free Software Foundation, Inc.

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

#define MC_TSTATE	0
#define MC_PC		1
#define MC_NPC		2
#define MC_Y		3
#define MC_G1		4
#define MC_G2		5
#define MC_G3		6
#define MC_G4		7
#define MC_G5		8
#define MC_G6		9
#define MC_G7		10
#define MC_O0		11
#define MC_O1		12
#define MC_O2		13
#define MC_O3		14
#define MC_O4		15
#define MC_O5		16
#define MC_O6		17
#define MC_O7		18
#define MC_NGREG	19

typedef unsigned long mc_greg_t;
typedef mc_greg_t mc_gregset_t[MC_NGREG];

#define MC_MAXFPQ	16
struct mc_fq {
	unsigned long	*mcfq_addr;
	unsigned int	mcfq_insn;
};

struct mc_fpu {
	union {
		unsigned int	sregs[32];
		unsigned long	dregs[32];
		long double	qregs[16];
	} mcfpu_fregs;
	unsigned long	mcfpu_fsr;
	unsigned long	mcfpu_fprs;
	unsigned long	mcfpu_gsr;
	struct mc_fq	*mcfpu_fq;
	unsigned char	mcfpu_qcnt;
	unsigned char	mcfpu_qentsz;
	unsigned char	mcfpu_enab;
};
typedef struct mc_fpu mc_fpu_t;

typedef struct {
	mc_gregset_t	mc_gregs;
	mc_greg_t	mc_fp;
	mc_greg_t	mc_i7;
	mc_fpu_t	mc_fpregs;
} mcontext_t;

typedef struct ucontext {
	struct ucontext		*uc_link;
	unsigned long		uc_flags;
	unsigned long		__uc_sigmask;
	mcontext_t		uc_mcontext;
	stack_t			uc_stack;
	__sigset_t		uc_sigmask;
} ucontext_t;

/*
 * Location of the users' stored registers relative to R0.
 * Usage is as an index into a gregset_t array or as u.u_ar0[XX].
 */
#define REG_PSR (0)
#define REG_PC  (1)
#define REG_nPC (2)
#define REG_Y   (3)
#define REG_G1  (4)
#define REG_G2  (5)
#define REG_G3  (6)
#define REG_G4  (7)
#define REG_G5  (8)
#define REG_G6  (9)
#define REG_G7  (10)
#define REG_O0  (11)
#define REG_O1  (12)
#define REG_O2  (13)
#define REG_O3  (14)
#define REG_O4  (15)
#define REG_O5  (16)
#define REG_O6  (17)
#define REG_O7  (18)

/*
 * A gregset_t is defined as an array type for compatibility with the reference
 * source. This is important due to differences in the way the C language
 * treats arrays and structures as parameters.
 *
 * Note that NGREG is really (sizeof (struct regs) / sizeof (greg_t)),
 * but that the ABI defines it absolutely to be 21 (resp. 19).
 */


#define REG_ASI	(19)
#define REG_FPRS (20)

#define NGREG   21
typedef long greg_t;
typedef greg_t  gregset_t[NGREG];

/*
 * The following structures define how a register window can appear on the
 * stack. This structure is available (when required) through the `gwins'
 * field of an mcontext (nested within ucontext). SPARC_MAXWINDOW is the
 * maximum number of outstanding regiters window defined in the SPARC
 * architecture (*not* implementation).
 */
#define SPARC_MAXREGWINDOW	31	/* max windows in SPARC arch. */
struct  rwindow
  {
    greg_t rw_local[8];			/* locals */
    greg_t rw_in[8];			/* ins */
  };

#define rw_fp   rw_in[6]		/* frame pointer */
#define rw_rtn  rw_in[7]		/* return address */

typedef struct gwindows
  {
    int            wbcnt;
    int           *spbuf[SPARC_MAXREGWINDOW];
    struct rwindow wbuf[SPARC_MAXREGWINDOW];
  } gwindows_t;

/*
 * Floating point definitions.
 */

#define MAXFPQ	16	/* max # of fpu queue entries currently supported */

/*
 * struct fq defines the minimal format of a floating point instruction queue
 * entry. The size of entries in the floating point queue are implementation
 * dependent. The union FQu is guarenteed to be the first field in any ABI
 * conformant system implementation. Any additional fields provided by an
 * implementation should not be used applications designed to be ABI conformant. */

struct fpq
  {
    unsigned long *fpq_addr;		/* address */
    unsigned long fpq_instr;		/* instruction */
  };

struct fq
  {
    union				/* FPU inst/addr queue */
      {
        double whole;
        struct fpq fpq;
      } FQu;
  };

#define FPU_REGS_TYPE           unsigned
#define FPU_DREGS_TYPE          unsigned long long
#define V7_FPU_FSR_TYPE         unsigned
#define V9_FPU_FSR_TYPE         unsigned long long
#define V9_FPU_FPRS_TYPE        unsigned

typedef struct fpu
  {
    union {				/* FPU floating point regs */
      unsigned		fpu_regs[32];	/* 32 singles */
      double            fpu_dregs[32];	/* 32 doubles */
      long double	fpu_qregs[16];  /* 16 quads */
    } fpu_fr;
    struct fq       *fpu_q;		/* ptr to array of FQ entries */
    unsigned long   fpu_fsr;		/* FPU status register */
    unsigned char   fpu_qcnt;		/* # of entries in saved FQ */
    unsigned char   fpu_q_entrysize;	/* # of bytes per FQ entry */
    unsigned char   fpu_en;		/* flag signifying fpu in use */
  } fpregset_t;

#endif /* sys/ucontext.h */
