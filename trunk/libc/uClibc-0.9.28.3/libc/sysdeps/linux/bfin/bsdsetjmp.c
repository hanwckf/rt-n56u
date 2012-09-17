/*
 * setjmp for the Blackfin project
 *
 * Copyright (C) 2003,  Metrowerks
 * Based on code from Analog Devices.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 */
#include <setjmp.h>

int setjmp(jmp_buf env){
   __asm__ __volatile__("[--SP] = p0;\n\t"
       "p0 = r0;\n\t"
       "r0 = [SP++];\n\t"

       "[p0++] = r0;\n\t"	/* GP address registers */
       "[p0++] = p1;\n\t"
       "[p0++] = p2;\n\t"
       "[p0++] = p3;\n\t"
       "[p0++] = p4;\n\t"
       "[p0++] = p5;\n\t"

       "[p0++] = FP;\n\t"       /* frame pointer */
       "[p0++] = SP;\n\t"	/* stack pointer */

       "[p0++] = p0;\n\t"	/* data regs */
       "[p0++] = r1;\n\t"
       "[p0++] = r2;\n\t"
       "[p0++] = r3;\n\t"
       "[p0++] = r4;\n\t"
       "[p0++] = r5;\n\t"
       "[p0++] = r6;\n\t"
       "[p0++] = r7;\n\t"

       "r0 = ASTAT;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = LC0;\n\t"		/* loop counters */
       "[p0++] = r0;\n\t"
       "r0 = LC1;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = A0.w;\n\t"
       "[p0++] = r0;\n\t"
       "r0.l = A0.x;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = A1.w;\n\t"
       "[p0++] = r0;\n\t"
       "r0.l = A1.x;\n\t"
       "[p0++] = r0;\n\t"

	       			/* Dag regs */
       "r0 = i0;\n\t"		/* index registers */
       "[p0++] = r0;\n\t"
       "r0 = i1;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = i2;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = i3;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = m0;\n\t"		/* modifier registers */
       "[p0++] = r0;\n\t"
       "r0 = m1;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = m2;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = m3;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = l0;\n\t"	        /* length registers */
       "[p0++] = r0;\n\t"
       "r0 = l1;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = l2;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = l3;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = b0;\n\t"	        /* base registers */
       "[p0++] = r0;\n\t"
       "r0 = b1;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = b2;\n\t"
       "[p0++] = r0;\n\t"
       "r0 = b3;\n\t"
       "[p0++] = r0;\n\t"

       "r0 = RETS;\n\t"		/* store return address */
       "[p0++] = r0;\n\t"

       "r0 = 0;\n\t"
      :
      :
     );
return 0;
}
