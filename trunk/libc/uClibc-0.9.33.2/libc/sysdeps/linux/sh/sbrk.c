/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

extern void * __curbrk attribute_hidden;
extern int __init_brk (void) attribute_hidden;
extern void *_brk(void *ptr) attribute_hidden;

void *
sbrk(intptr_t increment)
{
    if (__init_brk () == 0)
    {
		char * tmp = (char*)__curbrk+increment;
		__curbrk = _brk(tmp);
		if (__curbrk == tmp)
			return tmp-increment;
		__set_errno(ENOMEM);
		return ((void *) -1);
    }
    return ((void *) -1);
}
libc_hidden_def(sbrk)
