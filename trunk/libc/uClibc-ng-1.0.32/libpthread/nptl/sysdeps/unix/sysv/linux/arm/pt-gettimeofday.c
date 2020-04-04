#include <sys/syscall.h>
#include <sys/time.h>

int gettimeofday (struct timeval *, struct timezone *) attribute_hidden;
_syscall2(int, gettimeofday, struct timeval *, tv, struct timezone *, tz)
