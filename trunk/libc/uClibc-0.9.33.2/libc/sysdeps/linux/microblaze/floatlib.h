/*
** libgcc support for software floating point.
** Copyright (C) 1991 by Pipeline Associates, Inc.  All rights reserved.
** Permission is granted to do *anything* you want with this file,
** commercial or otherwise, provided this message remains intact.  So there!
** I would appreciate receiving any updates/patches/changes that anyone
** makes, and am willing to be the repository for said changes (am I
** making a big mistake?).

Warning! Only single-precision is actually implemented.  This file
won't really be much use until double-precision is supported.

However, once that is done, this file might eventually become a
replacement for libgcc1.c.  It might also make possible
cross-compilation for an IEEE target machine from a non-IEEE
host such as a VAX.

If you'd like to work on completing this, please talk to rms@gnu.ai.mit.edu.

--> Double precision floating support added by James Carlson on 20 April 1998.

**
** Pat Wood
** Pipeline Associates, Inc.
** pipeline!phw@motown.com or
** sun!pipeline!phw or
** uunet!motown!pipeline!phw
**
** 05/01/91 -- V1.0 -- first release to gcc mailing lists
** 05/04/91 -- V1.1 -- added float and double prototypes and return values
**                  -- fixed problems with adding and subtracting zero
**                  -- fixed rounding in truncdfsf2
**                  -- fixed SWAP define and tested on 386
*/

/*
** The following are routines that replace the libgcc soft floating point
** routines that are called automatically when -msoft-float is selected.
** The support single and double precision IEEE format, with provisions
** for byte-swapped machines (tested on 386).  Some of the double-precision
** routines work at full precision, but most of the hard ones simply punt
** and call the single precision routines, producing a loss of accuracy.
** long long support is not assumed or included.
** Overall accuracy is close to IEEE (actually 68882) for single-precision
** arithmetic.  I think there may still be a 1 in 1000 chance of a bit
** being rounded the wrong way during a multiply.  I'm not fussy enough to
** bother with it, but if anyone is, knock yourself out.
**
** Efficiency has only been addressed where it was obvious that something
** would make a big difference.  Anyone who wants to do this right for
** best speed should go in and rewrite in assembler.
**
** I have tested this only on a 68030 workstation and 386/ix integrated
** in with -msoft-float.
*/

#ifndef __FLOAT_LIB_H__
#define __FLOAT_LIB_H__
/* the following deal with IEEE single-precision numbers */
#define EXCESS		126
#define SIGNBIT		0x80000000
#define HIDDEN		(1 << 23)
#define SIGN(fp)	((fp) & SIGNBIT)
#define EXP(fp)		(((fp) >> 23) & 0xFF)
#define MANT(fp)	(((fp) & 0x7FFFFF) | HIDDEN)
#define PACK(s,e,m)	((s) | ((e) << 23) | (m))

/* the following deal with IEEE double-precision numbers */
#define EXCESSD		1022
#define HIDDEND		(1 << 20)
#define EXPD(fp)	(((fp.l.upper) >> 20) & 0x7FF)
#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
#define MANTD(fp)	(((((fp.l.upper) & 0xFFFFF) | HIDDEND) << 10) | \
				(fp.l.lower >> 22))
#define HIDDEND_LL	((long long)1 << 52)
#define MANTD_LL(fp)	((fp.ll & (HIDDEND_LL-1)) | HIDDEND_LL)
#define PACKD_LL(s,e,m)	(((long long)((s)+((e)<<20))<<32)|(m))

/* define SWAP for 386/960 reverse-byte-order brain-damaged CPUs */
union double_long {
    double d;
#ifdef SWAP
    struct {
      unsigned long lower;
      long upper;
    } l;
#else
    struct {
      long upper;
      unsigned long lower;
    } l;
#endif
    long long ll;
};

union float_long
  {
    float f;
    long l;
  };

#endif

/* Functions defined in different files */

float __addsf3 (float, float);
float __subsf3 (float, float);
long __cmpsf2 (float, float);
float __mulsf3 (float, float);
float __divsf3 (float, float);
double __floatsidf (register long);
double __floatdidf (register long long);
float __floatsisf (register long );
float __floatdisf (register long long );
float __negsf2 (float);
double __negdf2 (double);
double __extendsfdf2 (float);
float __truncdfsf2 (double);
long __cmpdf2 (double, double);
long __fixsfsi (float);
long __fixdfsi (double);
long long __fixdfdi (double);
unsigned long __fixunsdfsi (double);
unsigned long long __fixunsdfdi (double);
double __adddf3 (double, double);
double __subdf3 (double, double);
double __muldf3 (double, double);
double __divdf3 (double, double);
int __gtdf2 (double, double);
int __gedf2 (double, double);
int __ltdf2 (double, double);
int __ledf2 (double, double);
int __eqdf2 (double, double);
int __nedf2 (double, double);
int __gtsf2 (float, float);
int __gesf2 (float, float);
int __ltsf2 (float, float);
int __lesf2 (float, float);
int __eqsf2 (float, float);
int __nesf2 (float, float);
