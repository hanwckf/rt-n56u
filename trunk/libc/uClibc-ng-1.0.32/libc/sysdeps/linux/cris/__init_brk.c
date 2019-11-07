/* From libc-5.3.12 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include "sysdep.h"

void * __curbrk attribute_hidden = 0;

extern int __init_brk (void) attribute_hidden;
int
__init_brk (void)
{
    if (__curbrk == 0) {
	    /* Notice that we don't need to save/restore the GOT
	     * register since that is not call clobbered by the syscall.
	     */
	    __asm__ ("clear.d $r10\n\t"
		 "movu.w " STR(__NR_brk) ",$r9\n\t"
		 "break 13\n\t"
		 "move.d $r10, %0"
		 : "=r" (__curbrk)
		 :
		 : "r9", "r10");

	    if (__curbrk == 0) {
		    __set_errno(ENOMEM);
		    return -1;
	    }
    }
    return 0;
}
