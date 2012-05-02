
#include <errno.h>
#include <asm/ptrace.h>
#include <sys/syscall.h>

int
ptrace(int request, int pid, int addr, int data)
{
	long ret;
	long res;
	if (request > 0 && request < 4) data = (int)&ret;


	__asm__ volatile ("movel %1,%/d0\n\t"
			  "movel %2,%/d1\n\t"
			  "movel %3,%/d2\n\t"
			  "movel %4,%/d3\n\t"
			  "movel %5,%/d4\n\t"
			  "trap  #0\n\t"
			  "movel %/d0,%0"
		:"=g" (res)
		:"i" (__NR_ptrace), "g" (request), "g" (pid),
		 "g" (addr), "g" (data) : "%d0", "%d1", "%d2", "%d3", "%d4");

	if (res >= 0) {
		if (request > 0 && request < 4) {
			__set_errno(0);
			return (ret);
		}
		return (int) res;
	}
	__set_errno(-res);
	return -1;
}
