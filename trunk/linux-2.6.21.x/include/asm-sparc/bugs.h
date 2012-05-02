/*  $Id: bugs.h,v 1.1.1.1 2007-05-25 06:50:13 bruce Exp $
 *  include/asm-sparc/bugs.h:  Sparc probes for various bugs.
 *
 *  Copyright (C) 1996 David S. Miller (davem@caip.rutgers.edu)
 */

#include <asm/cpudata.h>

extern unsigned long loops_per_jiffy;

static void check_bugs(void)
{
#ifndef CONFIG_SMP
	cpu_data(0).udelay_val = loops_per_jiffy;
#endif
}
