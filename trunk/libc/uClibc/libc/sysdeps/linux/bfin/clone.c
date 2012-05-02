/*
 * libc/sysdeps/linux/bfin/clone.c -- `clone' syscall for linux/blackfin
 *
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 */

#include <asm/unistd.h>

int
clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg)
{
	register long rval = -1;

	if (fn && child_stack) {

	__asm__ __volatile__ (
			"r1 = %2;"
			"r0 = %3;"
			"P0 = %1;"
			"excpt 0;"	 /*Call sys_clone*/
			"%0  = r0;"
			"cc = r0 == 0;"
			"if !cc jump xxx;"	/* if (rval != 0) skip to parent */
			"r0 = %4;"
			"p0 = %5;"
			"call (p0);"	/* Call cloned function */
			"p0 = %6;"
			"excpt 0;"	/* Call sys_exit */
			"xxx: nop;"
			: "=d" (rval)
			: "i" (__NR_clone), "a" (child_stack), "a" (flags), "a" (arg), "a" (fn), "i" (__NR_exit)
			: "CC", "R0", "R1", "P0");
	}
	return rval;
}
