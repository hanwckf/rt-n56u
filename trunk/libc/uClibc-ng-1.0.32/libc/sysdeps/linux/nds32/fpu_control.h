/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _FPU_CONTROL_H

#ifdef  __NDS32_ABI_2FP_PLUS__
/*
 * Andes Floating-Point Control Status Register
 * 31-20 -> Reserved
 * 19	 -> RIT (RO)
 * 18	 -> DNIT(RO)
 * 17	 -> IEXT(RO)
 * 16	 -> UDFT(RO)
 * 15	 -> OVFT(RO)
 * 14	 -> DBZT(RO)
 * 13	 -> IVOT(RO)
 * 12	 -> DNZ(RW),Denormalized flush-to-Zero mode.
 * 11	 -> IEXE(RW),IEEE Ineaxct (IEX) exception trapping enable.
 * 10	 -> UDFE(RW),IEEE Underflow (UDF) exception trapping enable.
 * 9	 -> OVFE(RW),IEEE Overflow (OVF) exception trapping enable.
 * 8	 -> DBZE(RW),IEEE Divide by Zero (DBZ) exception trapping enable.
 * 7	 -> IVOE(RW),IEEE Invalid Operation (IVO) exception trapping enable.
 * 6	 -> IEX(RW),IEEE Inexact (IEX) cumulative exception flag.
 * 5	 -> UDF(RW),IEEE Underflow (UDF) cumulative exception flag.
 * 4	 -> OVF(RW),IEEE Overflow (OVF) cumulative exception flag.
 * 3	 -> DBZ(RW),IEEE Divide by Zero (DBZ) cumulative exception flag.
 * 2	 -> IVO(RW),IEEE Invalid Operation (IVO) cumulative exception flag.
 * 1-0	 -> Rounding modes.
 *
 * Rounding modes.
 * 00 - rounding to nearest (RN)
 * 01 - rounding (up) toward plus infinity (RP)
 * 10 - rounding (down)toward minus infinity (RM)
 * 11 - rounding toward zero (RZ)
 *
 */


/* masking of interrupts */
#define _FPU_MASK_IEX  	0x0800  /* Invalid operation */
#define _FPU_MASK_UDF  	0x0400  /* Underflow         */
#define _FPU_MASK_OVF  	0x0200  /* Overflow          */
#define _FPU_MASK_DBZ  	0x0100  /* Division by zero  */
#define _FPU_MASK_IVO 	0x0080  /* Invalid operation */

/*Reserved and read-only bits*/
#define _FPU_RESERVED	0xffffe000
#define _FPU_DEFAULT    0x00000000

/* Default + exceptions enabled. */
#define _FPU_IEEE	(_FPU_DEFAULT | 0x00000f80)

/* Type of the control word.  */
typedef unsigned int fpu_control_t;

/* Macros for accessing the hardware control word.  */
/* This is fmrx %0, fpscr.  */
#define _FPU_GETCW(cw) \
  __asm__ __volatile__ ("fmfcsr\t %0\n\t" : "=r" (cw))
/* This is fmxr fpscr, %0.  */
#define _FPU_SETCW(cw) \
  __asm__ __volatile__ ("fmtcsr\t %0\n\t": : "r" (cw))

/* Default control word set at startup.  */
extern fpu_control_t __fpu_control;
#else
#define _FPU_GETCW(cw) (cw) = 0
#define _FPU_SETCW(cw) (void) (cw)
#define _FPU_RESERVED 0xffffffff
#define _FPU_DEFAULT  0x00000000
typedef unsigned int fpu_control_t;
extern fpu_control_t __fpu_control;

#endif //__NDS32_ABI_2FP_PLUS__
#endif //_FPU_CONTROL_H
