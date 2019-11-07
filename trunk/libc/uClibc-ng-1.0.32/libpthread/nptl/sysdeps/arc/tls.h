/* Definition for thread-local data handling. NPTL/ARC version.
 *
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _TLS_H
#define _TLS_H

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>
# include <list.h>
# include <sysdep.h>
# include <bits/kernel-features.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

typedef struct
{
  dtv_t *dtv;
  uintptr_t pointer_guard;
} tcbhead_t;

# define TLS_MULTIPLE_THREADS_IN_TCB 1

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>

.macro THREAD_SELF reg
	# struct pthread is just ahead of TCB
	sub     \reg, r25, TLS_PRE_TCB_SIZE
.endm

.macro SET_TP  tcb
	mov    r25, \tcb
.endm

#endif /* __ASSEMBLER__ */


/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT
#define HAVE___THREAD   1
#define HAVE_TLS_MODEL_ATTRIBUTE       1
/* Signal that TLS support is available.  */
# define USE_TLS       1

#ifndef __ASSEMBLER__

/* Get system call information.  */
# include <sysdep.h>

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
#ifndef TLS_TCB_SIZE
# define TLS_TCB_SIZE sizeof (tcbhead_t)
#endif

/* This is the size we need before TCB.  */
# define TLS_PRE_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN __alignof__ (struct pthread)

/* The TLS blocks start right after the TCB.  */
# define TLS_DTV_AT_TP	1

/* Get the thread descriptor definition.  */
# include <descr.h>

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  ((tcbhead_t *) (tcbp))->dtv = (dtvp) + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv) \
  (THREAD_DTV() = (dtv))

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp, secondcall)          \
  ({                                            \
	long result_var;			\
	__builtin_set_thread_pointer(tcbp);     \
	result_var = INTERNAL_SYSCALL (arc_settls, err, 1, (tcbp));	\
	INTERNAL_SYSCALL_ERROR_P (result_var, err)			\
	? "unknown error" : NULL;		\
   })

/* Return the address of the dtv for the current thread.
   TP points to TCB where 1st item is dtv pointer */
# define THREAD_DTV() \
  (((tcbhead_t *)__builtin_thread_pointer())->dtv)

/* Return the thread descriptor for the current thread.
   pthread sits right before TCB */
# define THREAD_SELF \
  ((struct pthread *)__builtin_thread_pointer() - 1)

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF \
  CONST_THREAD_AREA (32, sizeof (struct pthread))

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  descr->member
#define THREAD_GETMEM_NC(descr, member, idx) \
  descr->member[idx]
#define THREAD_SETMEM(descr, member, value) \
  descr->member = (value)
#define THREAD_SETMEM_NC(descr, member, idx, value) \
  descr->member[idx] = (value)

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do									     \
    { int __res								     \
	= atomic_exchange_rel (&THREAD_SELF->header.gscope_flag,	     \
			       THREAD_GSCOPE_FLAG_UNUSED);		     \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				     \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);   \
    }									     \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  do									     \
    {									     \
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;	     \
      atomic_write_barrier ();						     \
    }									     \
  while (0)
#define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
