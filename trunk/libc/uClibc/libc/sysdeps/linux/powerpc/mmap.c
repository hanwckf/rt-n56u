
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/syscall.h>

#define __syscall_clobbers \
	"r9", "r10", "r11", "r12"
#define __syscall_return(type) \
	return (__sc_err & 0x10000000 ? errno = __sc_ret, __sc_ret = -1 : 0), \
	       (type) __sc_ret

void * mmap(void *start, size_t length, int prot, int flags, int fd,
	off_t offset)
{
	unsigned long __sc_ret, __sc_err;
	register unsigned long __sc_0 __asm__ ("r0");
	register unsigned long __sc_3 __asm__ ("r3");
	register unsigned long __sc_4 __asm__ ("r4");
	register unsigned long __sc_5 __asm__ ("r5");
	register unsigned long __sc_6 __asm__ ("r6");
	register unsigned long __sc_7 __asm__ ("r7");
	register unsigned long __sc_8 __asm__ ("r8");

	__sc_3 = (unsigned long) start;
	__sc_4 = (unsigned long) length;
	__sc_5 = (unsigned long) prot;
	__sc_6 = (unsigned long) flags;
	__sc_7 = (unsigned long) fd;
	__sc_8 = (unsigned long) offset;
	__sc_0 = __NR_mmap;
	__asm__ __volatile__
		("sc		\n\t"
		 "mfcr %1	"
		: "=&r" (__sc_3), "=&r" (__sc_0)
		: "0"   (__sc_3), "1"   (__sc_0),
		  "r"   (__sc_4),
		  "r"   (__sc_5),
		  "r"   (__sc_6),
		  "r"   (__sc_7),
		  "r"   (__sc_8)
		: __syscall_clobbers);
	__sc_ret = __sc_3;
	__sc_err = __sc_0;

	__syscall_return (void *);
}

