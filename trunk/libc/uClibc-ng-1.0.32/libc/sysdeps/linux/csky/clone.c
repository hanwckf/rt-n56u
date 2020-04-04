/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <stdarg.h>
#include <sysdep.h>
#include <unistd.h>

extern int __syscall_error(int err_no);

extern int __csky_clone (
  int flags,
  void *child_stack,
  pid_t *ptid,
  pid_t *ctid,
  void *tls);

int __clone(
  int (*fn)(void *),
  void *child_stack,
  int flags,
  void *arg, ...)
{
  void *ptid;
  void *tls;
  void *ctid;
  va_list al;
  int err;

  va_start(al, arg);
  ptid = va_arg(al, void *);
  tls = va_arg(al, void *);
  ctid = va_arg(al, void *);
  va_end(al);

  err = EINVAL;
  if (!fn)
    goto err;
  if (!child_stack)
    goto err;

  /* prepare fn&arg in child_stack */
  child_stack = (void *)((unsigned int)child_stack - 8);
  *(unsigned int *)child_stack = (unsigned int)fn;
  *(unsigned int *)(child_stack + 4) = (unsigned int)arg;

  return __csky_clone(flags, child_stack, ptid, ctid, tls);
err:
  return __syscall_error(-err);
}
weak_alias(__clone, clone)
