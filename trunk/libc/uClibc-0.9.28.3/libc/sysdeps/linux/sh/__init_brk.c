/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

void * __curbrk = 0;

#define __NR__brk __NR_brk
_syscall1(void *, _brk, void *, ptr);

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
