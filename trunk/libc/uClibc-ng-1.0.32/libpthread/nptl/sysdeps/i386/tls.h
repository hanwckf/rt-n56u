/* Definition for thread-local data handling.  nptl/i386 version.
   Copyright (C) 2002-2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _TLS_H
#define _TLS_H	1

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
  void *tcb;		/* Pointer to the TCB.  Not necessarily the
			   thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;		/* Pointer to the thread descriptor.  */
  int multiple_threads;
  uintptr_t sysinfo;
  uintptr_t stack_guard;
  uintptr_t pointer_guard;
  int gscope_flag;
#ifndef __ASSUME_PRIVATE_FUTEX
  int private_futex;
#else
  int __unused1;
#endif
  /* Reservation of some values for the TM ABI.  */
  void *__private_tm[5];
} tcbhead_t;

# define TLS_MULTIPLE_THREADS_IN_TCB 1

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif


/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT
#define HAVE___THREAD 1
#define HAVE_TLS_MODEL_ATTRIBUTE 1

/* Signal that TLS support is available.  */
#define USE_TLS        1


/* Alignment requirement for the stack.  For IA-32 this is governed by
   the SSE memory functions.  */
#define STACK_ALIGN	16

#ifndef __ASSEMBLER__
/* Get system call information.  */
# include <sysdep.h>

/* The old way: using LDT.  */

/* Structure passed to `modify_ldt', 'set_thread_area', and 'clone' calls.  */
struct user_desc
{
  unsigned int entry_number;
  unsigned long int base_addr;
  unsigned int limit;
  unsigned int seg_32bit:1;
  unsigned int contents:2;
  unsigned int read_exec_only:1;
  unsigned int limit_in_pages:1;
  unsigned int seg_not_present:1;
  unsigned int useable:1;
  unsigned int empty:25;
};

/* Initializing bit fields is slow.  We speed it up by using a union.  */
union user_desc_init
{
  struct user_desc desc;
  unsigned int vals[4];
};


/* Get the thread descriptor definition.  */
# include <descr.h>

/* This is the size of the initial TCB.  Can't be just sizeof (tcbhead_t),
   because NPTL getpid, __libc_alloca_cutoff etc. need (almost) the whole
   struct pthread even when not linked with -lpthread.  */
# define TLS_INIT_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (struct pthread)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN __alignof__ (struct pthread)

/* The TCB can have any size and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.  */
# define TLS_TCB_AT_TP	1


/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(descr, dtvp) \
  ((tcbhead_t *) (descr))->dtv = (dtvp) + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtvp) \
  ({ struct pthread *__pd;						      \
     THREAD_SETMEM (__pd, header.dtv, (dtvp)); })

/* Return dtv of given thread descriptor.  */
# define GET_DTV(descr) \
  (((tcbhead_t *) (descr))->dtv)

#define THREAD_SELF_SYSINFO	THREAD_GETMEM (THREAD_SELF, header.sysinfo)
#define THREAD_SYSINFO(pd)	((pd)->header.sysinfo)

/* Macros to load from and store into segment registers.  */
# ifndef TLS_GET_GS
#  define TLS_GET_GS() \
  ({ int __seg; __asm__ ("movw %%gs, %w0" : "=q" (__seg)); __seg & 0xffff; })
# endif
# ifndef TLS_SET_GS
#  define TLS_SET_GS(val) \
  __asm__ ("movw %w0, %%gs" :: "q" (val))
# endif


# ifndef __NR_set_thread_area
#  define __NR_set_thread_area 243
# endif
# ifndef TLS_FLAG_WRITABLE
#  define TLS_FLAG_WRITABLE		0x00000001
# endif

// XXX Enable for the real world.
#if 0
# ifndef __ASSUME_SET_THREAD_AREA
#  error "we need set_thread_area"
# endif
#endif

# ifdef __PIC__
#  define TLS_EBX_ARG "r"
#  define TLS_LOAD_EBX "xchgl %3, %%ebx\n\t"
# else
#  define TLS_EBX_ARG "b"
#  define TLS_LOAD_EBX
# endif

#if defined NEED_DL_SYSINFO
# define INIT_SYSINFO \
  _head->sysinfo = GLRO(dl_sysinfo)
#else
# define INIT_SYSINFO
#endif

#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX  /* nothing */
# else
#  define LOCK_PREFIX "lock;"
# endif
#endif

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(thrdescr, secondcall) \
  ({ void *_thrdescr = (thrdescr);					      \
     tcbhead_t *_head = _thrdescr;					      \
     union user_desc_init _segdescr;					      \
     int _result;							      \
									      \
     _head->tcb = _thrdescr;						      \
     /* For now the thread descriptor is at the same address.  */	      \
     _head->self = _thrdescr;						      \
     /* New syscall handling support.  */				      \
     INIT_SYSINFO;							      \
									      \
     /* The 'entry_number' field.  Let the kernel pick a value.  */	      \
     if (secondcall)							      \
       _segdescr.vals[0] = TLS_GET_GS () >> 3;				      \
     else								      \
       _segdescr.vals[0] = -1;						      \
     /* The 'base_addr' field.  Pointer to the TCB.  */			      \
     _segdescr.vals[1] = (unsigned long int) _thrdescr;			      \
     /* The 'limit' field.  We use 4GB which is 0xfffff pages.  */	      \
     _segdescr.vals[2] = 0xfffff;					      \
     /* Collapsed value of the bitfield:				      \
	  .seg_32bit = 1						      \
	  .contents = 0							      \
	  .read_exec_only = 0						      \
	  .limit_in_pages = 1						      \
	  .seg_not_present = 0						      \
	  .useable = 1 */						      \
     _segdescr.vals[3] = 0x51;						      \
									      \
     /* Install the TLS.  */						      \
     __asm__ __volatile__ (TLS_LOAD_EBX						  \
		   "int $0x80\n\t"					      \
		   TLS_LOAD_EBX						      \
		   : "=a" (_result), "=m" (_segdescr.desc.entry_number)	      \
		   : "0" (__NR_set_thread_area),			      \
		     TLS_EBX_ARG (&_segdescr.desc), "m" (_segdescr.desc));    \
									      \
     if (_result == 0)							      \
       /* We know the index in the GDT, now load the segment register.	      \
	  The use of the GDT is described by the value 3 in the lower	      \
	  three bits of the segment descriptor value.			      \
									      \
	  Note that we have to do this even if the numeric value of	      \
	  the descriptor does not change.  Loading the segment register	      \
	  causes the segment information from the GDT to be loaded	      \
	  which is necessary since we have changed it.   */		      \
       TLS_SET_GS (_segdescr.desc.entry_number * 8 + 3);		      \
									      \
     _result == 0 ? NULL						      \
     : "set_thread_area failed when setting up thread-local storage\n"; })


/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  ({ struct pthread *__pd;						      \
     THREAD_GETMEM (__pd, header.dtv); })


/* Return the thread descriptor for the current thread.

   The contained asm must *not* be marked __volatile__ since otherwise
   assignments like
        pthread_descr self = thread_self();
   do not get optimized away.  */
# define THREAD_SELF \
  ({ struct pthread *__self;						      \
     __asm__ ("movl %%gs:%c1,%0" : "=r" (__self)				    \
	  : "i" (offsetof (struct pthread, header.self)));		      \
     __self;})

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF \
  REGISTER_THREAD_AREA (32, offsetof (struct user_regs_struct, xgs), 3) \
  REGISTER_THREAD_AREA (64, 26 * 8, 3) /* x86-64's user_regs_struct->gs */


/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) \
  ({ __typeof (descr->member) __value;					      \
     if (sizeof (__value) == 1)						      \
       __asm__ __volatile__ ("movb %%gs:%P2,%b0"				    \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member)));     \
     else if (sizeof (__value) == 4)					      \
       __asm__ __volatile__ ("movl %%gs:%P1,%0"					    \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movl %%gs:%P1,%%eax\n\t"			      \
		       "movl %%gs:%P2,%%edx"				      \
		       : "=A" (__value)					      \
		       : "i" (offsetof (struct pthread, member)),	      \
			 "i" (offsetof (struct pthread, member) + 4));	      \
       }								      \
     __value; })


/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member, idx) \
  ({ __typeof (descr->member[0]) __value;				      \
     if (sizeof (__value) == 1)						      \
       __asm__ __volatile__ ("movb %%gs:%P2(%3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member[0])),   \
		     "r" (idx));					      \
     else if (sizeof (__value) == 4)					      \
       __asm__ __volatile__ ("movl %%gs:%P1(,%2,4),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member[0])),	      \
		       "r" (idx));					      \
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__  ("movl %%gs:%P1(,%2,8),%%eax\n\t"		      \
			"movl %%gs:4+%P1(,%2,8),%%edx"			      \
			: "=&A" (__value)				      \
			: "i" (offsetof (struct pthread, member[0])),	      \
			  "r" (idx));					      \
       }								      \
     __value; })


/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM(descr, member, value) \
  ({ if (sizeof (descr->member) == 1)					      \
       __asm__ __volatile__ ("movb %b0,%%gs:%P1" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member)));	      \
     else if (sizeof (descr->member) == 4)				      \
       __asm__ __volatile__ ("movl %0,%%gs:%P1" :				      \
		     : "ir" (value),					      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (descr->member) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movl %%eax,%%gs:%P1\n\t"			      \
		       "movl %%edx,%%gs:%P2" :				      \
		       : "A" (value),					      \
			 "i" (offsetof (struct pthread, member)),	      \
			 "i" (offsetof (struct pthread, member) + 4));	      \
       }})


/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM_NC(descr, member, idx, value) \
  ({ if (sizeof (descr->member[0]) == 1)				      \
       __asm__ __volatile__ ("movb %b0,%%gs:%P1(%2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member)),		      \
		       "r" (idx));					      \
     else if (sizeof (descr->member[0]) == 4)				      \
       __asm__ __volatile__ ("movl %0,%%gs:%P1(,%2,4)" :			      \
		     : "ir" (value),					      \
		       "i" (offsetof (struct pthread, member)),		      \
		       "r" (idx));					      \
     else								      \
       {								      \
	 if (sizeof (descr->member[0]) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movl %%eax,%%gs:%P1(,%2,8)\n\t"			      \
		       "movl %%edx,%%gs:4+%P1(,%2,8)" :			      \
		       : "A" (value),					      \
			 "i" (offsetof (struct pthread, member)),	      \
			 "r" (idx));					      \
       }})


/* Atomic compare and exchange on TLS, returning old value.  */
#define THREAD_ATOMIC_CMPXCHG_VAL(descr, member, newval, oldval) \
  ({ __typeof (descr->member) __ret;					      \
     __typeof (oldval) __old = (oldval);				      \
     if (sizeof (descr->member) == 4)					      \
       __asm__ __volatile__ (LOCK_PREFIX "cmpxchgl %2, %%gs:%P3"		      \
		     : "=a" (__ret)					      \
		     : "0" (__old), "r" (newval),			      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       /* Not necessary for other sizes in the moment.  */		      \
       abort ();							      \
     __ret; })


/* Atomic logical and.  */
#define THREAD_ATOMIC_AND(descr, member, val) \
  (void) ({ if (sizeof ((descr)->member) == 4)				      \
	      __asm__ __volatile__ (LOCK_PREFIX "andl %1, %%gs:%P0"		      \
			    :: "i" (offsetof (struct pthread, member)),	      \
			       "ir" (val));				      \
	    else							      \
	      /* Not necessary for other sizes in the moment.  */	      \
	      abort (); })


/* Atomic set bit.  */
#define THREAD_ATOMIC_BIT_SET(descr, member, bit) \
  (void) ({ if (sizeof ((descr)->member) == 4)				      \
	      __asm__ __volatile__ (LOCK_PREFIX "orl %1, %%gs:%P0"		      \
			    :: "i" (offsetof (struct pthread, member)),	      \
			       "ir" (1 << (bit)));			      \
	    else							      \
	      /* Not necessary for other sizes in the moment.  */	      \
	      abort (); })


/* Set the stack guard field in TCB head.  */
#define THREAD_SET_STACK_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, header.stack_guard, value)
#define THREAD_COPY_STACK_GUARD(descr) \
  ((descr)->header.stack_guard						      \
   = THREAD_GETMEM (THREAD_SELF, header.stack_guard))


/* Set the pointer guard field in the TCB head.  */
#define THREAD_SET_POINTER_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, header.pointer_guard, value)
#define THREAD_COPY_POINTER_GUARD(descr) \
  ((descr)->header.pointer_guard					      \
   = THREAD_GETMEM (THREAD_SELF, header.pointer_guard))


/* Get and set the global scope generation counter in the TCB head.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do									      \
    { int __res;							      \
      __asm__ __volatile__ ("xchgl %0, %%gs:%P1"				      \
		    : "=r" (__res)					      \
		    : "i" (offsetof (struct pthread, header.gscope_flag)),    \
		      "0" (THREAD_GSCOPE_FLAG_UNUSED));			      \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				      \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);    \
    }									      \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  THREAD_SETMEM (THREAD_SELF, header.gscope_flag, THREAD_GSCOPE_FLAG_USED)
#define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
