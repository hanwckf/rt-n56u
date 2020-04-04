/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

void * __curbrk attribute_hidden = 0;

#define __NR__brk __NR_brk
attribute_hidden _syscall1(void *, _brk, void *, ptr)

extern int __init_brk (void) attribute_hidden;
int
__init_brk (void)
{
    if (__curbrk == 0)
    {
		__curbrk = _brk(0);
		if (__curbrk == 0)
		{
		  __set_errno(ENOMEM);
		  return -1;
		}
    }
    return 0;
}
