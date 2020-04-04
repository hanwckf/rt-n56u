#include <sched.h>
#include <signal.h>
#include <sysdep.h>
#include <tls.h>

#define ARCH_FORK() \
  INLINE_SYSCALL (clone, 5,                                                  \
                 CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD, 0,     \
                 NULL, NULL, &THREAD_SELF->tid)

#include "../fork.c"
