/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <stdarg.h>

#if defined __NR_ptrace && defined __USE_BSD && defined __USE_MISC
#define __NR___syscall_ptrace __NR_ptrace

static __inline__ _syscall4(long, __syscall_ptrace, enum __ptrace_request, request,
		__kernel_pid_t, pid, void*, addr, void*, data)

long int
ptrace (enum __ptrace_request request, ...)
{
  long int res, ret;
  va_list ap;
  pid_t pid;
  void *addr, *data;

  va_start (ap, request);
  pid = va_arg (ap, pid_t);
  addr = va_arg (ap, void *);
  data = va_arg (ap, void *);
  va_end (ap);

  if (request > 0 && request < 4)
    data = &ret;

  res = __syscall_ptrace(request, pid, addr, data);
  if (res >= 0 && request > 0 && request < 4) {
      __set_errno(0);
      return ret;
    }

  return res;
}
#endif
