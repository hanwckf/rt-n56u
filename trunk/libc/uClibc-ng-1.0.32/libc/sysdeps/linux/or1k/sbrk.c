/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
libc_hidden_proto(sbrk)

extern void * ___brk_addr;

extern int __init_brk (void);
extern void *_brk(void *ptr);

void *
sbrk(intptr_t increment)
{
    if (__init_brk () == 0)
    {
		char * tmp = (char*)___brk_addr+increment;
		___brk_addr = _brk(tmp);
		if (___brk_addr == tmp)
			return tmp-increment;
		__set_errno(ENOMEM);
		return ((void *) -1);
    }
    return ((void *) -1);
}
libc_hidden_def(sbrk)
