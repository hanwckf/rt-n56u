/* Definition for thread-local data handling.  nptl/x86_64 version.
   Copyright (C) 2002-2007, 2008, 2009 Free Software Foundation, Inc.
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
# include <asm/prctl.h>	/* For ARCH_SET_FS.  */
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>
# include <sysdep.h>
# include <bits/kernel-features.h>
# include <bits/wordsize.h>
# include <xmmintrin.h>


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
  int gscope_flag;
  uintptr_t sysinfo;
  uintptr_t stack_guard;
  uintptr_t pointer_guard;
  unsigned long int vgetcpu_cache[2];
# ifndef __ASSUME_PRIVATE_FUTEX
  int private_futex;
# else
  int __unused1;
# endif
# if __WORDSIZE == 64
  int rtld_must_xmm_save;
# endif
  /* Reservation of some values for the TM ABI.  */
  void *__private_tm[5];
# if __WORDSIZE == 64
  long int __unused2;
  /* Have space for the post-AVX register size.  */
  __m128 rtld_savespace_sse[8][4];

  void *__padding[8];
# endif
} tcbhead_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif


/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT                1
#define HAVE___THREAD 1
#define HAVE_TLS_MODEL_ATTRIBUTE 1

/* Signal that TLS support is available.  */
#define USE_TLS        1

/* Alignment requirement for the stack.  */
#define STACK_ALIGN	16


#ifndef __ASSEMBLER__
/* Get system call information.  */
# include <sysdep.h>


/* Get the thread descriptor definition.  */
# include <descr.h>

#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX	/* nothing */
# else
#  define LOCK_PREFIX	"lock;"
# endif
#endif

/* This is the size of the initial TCB.  Can't be just sizeof (tcbhead_t),
   because NPTL getpid, __libc_alloca_cutoff etc. need (almost) the whole
   struct pthread even when not linked with -lpthread.  */
# define TLS_INIT_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (struct pthread)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
//# define TLS_TCB_ALIGN __alignof__ (struct pthread)
// Normally the above would be correct  But we have to store post-AVX
// vector registers in the TCB and we want the storage to be aligned.
// unfortunately there isn't yet a type for these values and hence no
// 32-byte alignment requirement.  Make this explicit, for now.
# define TLS_TCB_ALIGN 32

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


/* Macros to load from and store into segment registers.  */
# define TLS_GET_FS() \
  ({ int __seg; __asm__ ("movl %%fs, %0" : "=q" (__seg)); __seg; })
# define TLS_SET_FS(val) \
  __asm__ ("movl %0, %%fs" :: "q" (val))


/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.

   We have to make the syscall for both uses of the macro since the
   address might be (and probably is) different.  */
# define TLS_INIT_TP(thrdescr, secondcall) \
  ({ void *_thrdescr = (thrdescr);					      \
     tcbhead_t *_head = _thrdescr;					      \
     int _result;							      \
									      \
     _head->tcb = _thrdescr;						      \
     /* For now the thread descriptor is at the same address.  */	      \
     _head->self = _thrdescr;						      \
									      \
     /* It is a simple syscall to set the %fs value for the thread.  */	      \
     __asm__ __volatile__ ("syscall"						      \
		   : "=a" (_result)					      \
		   : "0" ((unsigned long int) __NR_arch_prctl),		      \
		     "D" ((unsigned long int) ARCH_SET_FS),		      \
		     "S" (_thrdescr)					      \
		   : "memory", "cc", "r11", "cx");			      \
									      \
    _result ? "cannot set %fs base address for thread-local storage" : 0;     \
  })


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
     __asm__ ("movq %%fs:%c1,%q0" : "=r" (__self)				      \
	  : "i" (offsetof (struct pthread, header.self)));	 	      \
     __self;})

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF_INCLUDE  <sys/reg.h> /* For the FS constant.  */
# define DB_THREAD_SELF CONST_THREAD_AREA (64, FS)

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) \
  ({ __typeof (descr->member) __value;					      \
     if (sizeof (__value) == 1)						      \
       __asm__ __volatile__ ("movb %%fs:%P2,%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member)));     \
     else if (sizeof (__value) == 4)					      \
       __asm__ __volatile__ ("movl %%fs:%P1,%0"					      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movq %%fs:%P1,%q0"				      \
		       : "=r" (__value)					      \
		       : "i" (offsetof (struct pthread, member)));	      \
       }								      \
     __value; })


/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member, idx) \
  ({ __typeof (descr->member[0]) __value;				      \
     if (sizeof (__value) == 1)						      \
       __asm__ __volatile__ ("movb %%fs:%P2(%q3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member[0])),   \
		       "r" (idx));					      \
     else if (sizeof (__value) == 4)					      \
       __asm__ __volatile__ ("movl %%fs:%P1(,%q2,4),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member[0])), "r" (idx));\
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movq %%fs:%P1(,%q2,8),%q0"			      \
		       : "=r" (__value)					      \
		       : "i" (offsetof (struct pthread, member[0])),	      \
			 "r" (idx));					      \
       }								      \
     __value; })


/* Loading addresses of objects on x86-64 needs to be treated special
   when generating PIC code.  */
#ifdef __pic__
# define IMM_MODE "nr"
#else
# define IMM_MODE "ir"
#endif


/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM(descr, member, value) \
  ({ if (sizeof (descr->member) == 1)					      \
       __asm__ __volatile__ ("movb %b0,%%fs:%P1" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member)));	      \
     else if (sizeof (descr->member) == 4)				      \
       __asm__ __volatile__ ("movl %0,%%fs:%P1" :				      \
		     : IMM_MODE (value),				      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (descr->member) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movq %q0,%%fs:%P1" :				      \
		       : IMM_MODE ((unsigned long int) value),		      \
			 "i" (offsetof (struct pthread, member)));	      \
       }})


/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM_NC(descr, member, idx, value) \
  ({ if (sizeof (descr->member[0]) == 1)				      \
       __asm__ __volatile__ ("movb %b0,%%fs:%P1(%q2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member[0])),	      \
		       "r" (idx));					      \
     else if (sizeof (descr->member[0]) == 4)				      \
       __asm__ __volatile__ ("movl %0,%%fs:%P1(,%q2,4)" :			      \
		     : IMM_MODE (value),				      \
		       "i" (offsetof (struct pthread, member[0])),	      \
		       "r" (idx));					      \
     else								      \
       {								      \
	 if (sizeof (descr->member[0]) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 __asm__ __volatile__ ("movq %q0,%%fs:%P1(,%q2,8)" :			      \
		       : IMM_MODE ((unsigned long int) value),		      \
			 "i" (offsetof (struct pthread, member[0])),	      \
			 "r" (idx));					      \
       }})


/* Atomic compare and exchange on TLS, returning old value.  */
# define THREAD_ATOMIC_CMPXCHG_VAL(descr, member, newval, oldval) \
  ({ __typeof (descr->member) __ret;					      \
     __typeof (oldval) __old = (oldval);				      \
     if (sizeof (descr->member) == 4)					      \
       __asm__ __volatile__ (LOCK_PREFIX "cmpxchgl %2, %%fs:%P3"		      \
		     : "=a" (__ret)					      \
		     : "0" (__old), "r" (newval),			      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       /* Not necessary for other sizes in the moment.  */		      \
       abort ();							      \
     __ret; })


/* Atomic logical and.  */
# define THREAD_ATOMIC_AND(descr, member, val) \
  (void) ({ if (sizeof ((descr)->member) == 4)				      \
	      __asm__ __volatile__ (LOCK_PREFIX "andl %1, %%fs:%P0"		      \
			    :: "i" (offsetof (struct pthread, member)),	      \
			       "ir" (val));				      \
	    else							      \
	      /* Not necessary for other sizes in the moment.  */	      \
	      abort (); })


/* Atomic set bit.  */
# define THREAD_ATOMIC_BIT_SET(descr, member, bit) \
  (void) ({ if (sizeof ((descr)->member) == 4)				      \
	      __asm__ __volatile__ (LOCK_PREFIX "orl %1, %%fs:%P0"		      \
			    :: "i" (offsetof (struct pthread, member)),	      \
			       "ir" (1 << (bit)));			      \
	    else							      \
	      /* Not necessary for other sizes in the moment.  */	      \
	      abort (); })


/* Set the stack guard field in TCB head.  */
# define THREAD_SET_STACK_GUARD(value) \
    THREAD_SETMEM (THREAD_SELF, header.stack_guard, value)
# define THREAD_COPY_STACK_GUARD(descr) \
    ((descr)->header.stack_guard					      \
     = THREAD_GETMEM (THREAD_SELF, header.stack_guard))


/* Set the pointer guard field in the TCB head.  */
# define THREAD_SET_POINTER_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, header.pointer_guard, value)
# define THREAD_COPY_POINTER_GUARD(descr) \
  ((descr)->header.pointer_guard					      \
   = THREAD_GETMEM (THREAD_SELF, header.pointer_guard))


/* Get and set the global scope generation counter in the TCB head.  */
# define THREAD_GSCOPE_FLAG_UNUSED 0
# define THREAD_GSCOPE_FLAG_USED   1
# define THREAD_GSCOPE_FLAG_WAIT   2
# define THREAD_GSCOPE_RESET_FLAG() \
  do									      \
    { int __res;							      \
      __asm__ __volatile__ ("xchgl %0, %%fs:%P1"				      \
		    : "=r" (__res)					      \
		    : "i" (offsetof (struct pthread, header.gscope_flag)),    \
		      "0" (THREAD_GSCOPE_FLAG_UNUSED));			      \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				      \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);    \
    }									      \
  while (0)
# define THREAD_GSCOPE_SET_FLAG() \
  THREAD_SETMEM (THREAD_SELF, header.gscope_flag, THREAD_GSCOPE_FLAG_USED)
# define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()


# ifdef SHARED
/* Defined in dl-trampoline.S.  */
extern void _dl_x86_64_save_sse (void);
extern void _dl_x86_64_restore_sse (void);

# define RTLD_CHECK_FOREIGN_CALL \
  (THREAD_GETMEM (THREAD_SELF, header.rtld_must_xmm_save) != 0)

/* NB: Don't use the xchg operation because that would imply a lock
   prefix which is expensive and unnecessary.  The cache line is also
   not contested at all.  */
#  define RTLD_ENABLE_FOREIGN_CALL \
  int old_rtld_must_xmm_save = THREAD_GETMEM (THREAD_SELF,		      \
					      header.rtld_must_xmm_save);     \
  THREAD_SETMEM (THREAD_SELF, header.rtld_must_xmm_save, 1)

#  define RTLD_PREPARE_FOREIGN_CALL \
  do if (THREAD_GETMEM (THREAD_SELF, header.rtld_must_xmm_save))	      \
    {									      \
      _dl_x86_64_save_sse ();						      \
      THREAD_SETMEM (THREAD_SELF, header.rtld_must_xmm_save, 0);	      \
    }									      \
  while (0)

#  define RTLD_FINALIZE_FOREIGN_CALL \
  do {									      \
    if (THREAD_GETMEM (THREAD_SELF, header.rtld_must_xmm_save) == 0)	      \
      _dl_x86_64_restore_sse ();					      \
    THREAD_SETMEM (THREAD_SELF, header.rtld_must_xmm_save,		      \
		   old_rtld_must_xmm_save);				      \
  } while (0)
# endif


#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
