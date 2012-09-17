/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

extern void * __curbrk;

extern int __init_brk (void);
extern void *_brk(void *ptr);

int brk(void * end_data_seg)
{
    if (__init_brk () == 0)
    {
		__curbrk = _brk(end_data_seg);
		if (__curbrk == end_data_seg)
			return 0;
		__set_errno(ENOMEM);
    }
    return -1;
}
