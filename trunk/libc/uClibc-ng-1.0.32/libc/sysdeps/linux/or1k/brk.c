/* From libc-5.3.12 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
libc_hidden_proto(brk)
extern void * ___brk_addr;

extern int __init_brk (void);
extern void *_brk(void *ptr);

int brk(void * end_data_seg)
{
    if (__init_brk () == 0)
    {
		___brk_addr = _brk(end_data_seg);
		if (___brk_addr == end_data_seg)
			return 0;
		__set_errno(ENOMEM);
    }
    return -1;
}
libc_hidden_def(brk)
