/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

void * ___brk_addr = 0;

int __init_brk (void);
void *_brk(void *ptr);

#define __NR__brk __NR_brk
_syscall1(void *, _brk, void *, ptr);

int
__init_brk (void)
{
    if (___brk_addr == 0)
    {
		___brk_addr = _brk(0);
		if (___brk_addr == 0)
		{
		  __set_errno(ENOMEM);
		  return -1;
		}
    }
    return 0;
}
