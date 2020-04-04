/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _TLS_H
#define _TLS_H 1

#ifndef __ASSEMBLER__

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

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

#ifdef __CSKYABIV2__
/* define r31 as thread pointer register? */
# define READ_THREAD_POINTER() \
    ({ void *__result;                                \
       __asm__ __volatile__ ("mov %0, r31"	          \
                             : "=r" (__result));      \
       __result; })
#else
# define READ_THREAD_POINTER() \
    ({ register unsigned int __result __asm__("a0");                 \
       __asm__ __volatile__ ("trap 3;"	    \
                             : "=r" (__result) : : );      \
       __result; })
#endif

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
# ifdef __CSKYABIV2__
/* define r31 as thread pointer register? */
# define READ_THREAD_POINTER() \
       mov r0, r31;
# else
# define READ_THREAD_POINTER() \
       trap 3;
# endif
#endif /* __ASSEMBLER__ */

/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT                1
#define HAVE_TLS_MODEL_ATTRIBUTE        1
#define HAVE___THREAD                   1

/* Signal that TLS support is available.  */
#define USE_TLS	1

# ifndef __ASSEMBLER__

/* Get system call information.  */
#  include <sysdep.h>

/* The TP points to the start of the thread blocks.  */
# define TLS_DTV_AT_TP	1

/* Get the thread descriptor definition.  */
# include <../../descr.h>

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE	     sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN     16

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE		sizeof (tcbhead_t)

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN	    16

/* This is the size we need before TCB.  */
#  define TLS_PRE_TCB_SIZE	sizeof (struct pthread)

/* The thread pointer (in hardware register $29) points to the end of
   the TCB + 0x7000, as for PowerPC.  The pthread_descr structure is
   immediately in front of the TCB.  */
# define TLS_TCB_OFFSET     0//0x7000

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(tcbp, dtvp) \
  (((tcbhead_t *) (tcbp))->dtv = (dtvp) + 1)

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(dtv) \
  (THREAD_DTV() = (dtv))

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp, secondcall) \
  ({ INTERNAL_SYSCALL_DECL (err);                   \
     long result_var;                           \
     result_var = INTERNAL_SYSCALL (set_thread_area, err, 1,        \
                    (char *) (tcbp) + TLS_TCB_OFFSET);  \
     INTERNAL_SYSCALL_ERROR_P (result_var, err)             \
       ? "unknown error" : NULL; })

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  (((tcbhead_t *) (READ_THREAD_POINTER () - TLS_TCB_OFFSET))->dtv)

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
 ((struct pthread *) (READ_THREAD_POINTER ()                 \
              - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

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

/* Initializing the thread pointer will generate a SIGILL if the syscall
   is not available.  */
#define TLS_INIT_TP_EXPENSIVE 1

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
