/* Machine-dependent pthreads configuration and inline functions.
   x86-64 version.
   Copyright (C) 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

# include <features.h>

#ifndef __ASSEMBLER__
# include <stddef.h>	/* For offsetof.  */
# include <stdlib.h>	/* For abort().  */
# include <asm/prctl.h>


# ifndef PT_EI
#  define PT_EI __extern_always_inline
# endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
# define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%rsp") __attribute_used__;


/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret;

  __asm__ __volatile__ (
	"xchgl %k0, %1"
	: "=r"(ret), "=m"(*spinlock)
	: "0"(1), "m"(*spinlock)
	: "memory");

  return ret;
}


/* Compare-and-swap for semaphores.  */
# define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("lock; cmpxchgq %3, %1; sete %0"
			: "=q" (ret), "=m" (*p), "=a" (readval)
			: "r" (newval), "m" (*p), "a" (oldval)
			: "memory");
  return ret;
}

/* Return the thread descriptor for the current thread.

   The contained asm must *not* be marked volatile since otherwise
   assignments like
	pthread_descr self = thread_self();
   do not get optimized away.  */
# define THREAD_SELF \
({									      \
  register pthread_descr __self;					      \
  __asm__ ("movq %%fs:%c1,%0" : "=r" (__self)				      \
	   : "i" (offsetof (struct _pthread_descr_struct,		      \
			    p_header.data.self)));			      \
  __self;								      \
})

/* Prototype for the system call.  */
extern int arch_prctl (int __code, unsigned long __addr);

/* Initialize the thread-unique value.  */
# define INIT_THREAD_SELF(descr, nr) \
{									      \
  if (arch_prctl (ARCH_SET_FS, (unsigned long)descr) != 0)		      \
    abort ();								      \
}

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) \
({									      \
  __typeof__ (descr->member) __value;					      \
  if (sizeof (__value) == 1)						      \
    __asm__ __volatile__ ("movb %%fs:%P2,%b0"				      \
			  : "=q" (__value)				      \
			  : "0" (0),					      \
			    "i" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else if (sizeof (__value) == 4)					      \
    __asm__ __volatile__ ("movl %%fs:%P2,%k0"				      \
			  : "=r" (__value)				      \
			  : "0" (0),					      \
			    "i" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else									      \
    {									      \
      if (sizeof (__value) != 8)					      \
	/* There should not be any value with a size other than 1, 4 or 8.  */\
	abort ();							      \
									      \
      __asm__ __volatile__ ("movq %%fs:%P1,%0"				      \
			    : "=r" (__value)				      \
			    : "i" (offsetof (struct _pthread_descr_struct,    \
					     member)));			      \
    }									      \
  __value;								      \
})

/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member) \
({									      \
  __typeof__ (descr->member) __value;					      \
  if (sizeof (__value) == 1)						      \
    __asm__ __volatile__ ("movb %%fs:(%2),%b0"				      \
			  : "=q" (__value)				      \
			  : "0" (0),					      \
			    "r" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else if (sizeof (__value) == 4)					      \
    __asm__ __volatile__ ("movl %%fs:(%2),%k0"				      \
			  : "=r" (__value)				      \
			  : "0" (0),					      \
			    "r" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else									      \
    {									      \
      if (sizeof (__value) != 8)					      \
	/* There should not be any value with a size other than 1, 4 or 8.  */\
	abort ();							      \
									      \
      __asm__ __volatile__ ("movq %%fs:(%1),%0"				      \
			    : "=r" (__value)				      \
			    : "r" (offsetof (struct _pthread_descr_struct,    \
					     member)));			      \
    }									      \
  __value;								      \
})

/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM(descr, member, value) \
({									      \
  __typeof__ (descr->member) __value = (value);				      \
  if (sizeof (__value) == 1)						      \
    __asm__ __volatile__ ("movb %0,%%fs:%P1" :				      \
			  : "q" (__value),				      \
			    "i" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else if (sizeof (__value) == 4)					      \
    __asm__ __volatile__ ("movl %k0,%%fs:%P1" :				      \
			  : "r" (__value),				      \
			    "i" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else									      \
    {									      \
      if (sizeof (__value) != 8)					      \
	/* There should not be any value with a size other than 1, 4 or 8.  */\
	abort ();							      \
									      \
      __asm__ __volatile__ ("movq %0,%%fs:%P1" :			      \
			    : "r" (__value),				      \
			      "i" (offsetof (struct _pthread_descr_struct,    \
					     member)));			      \
    }									      \
})

/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM_NC(descr, member, value) \
({									      \
  __typeof__ (descr->member) __value = (value);				      \
  if (sizeof (__value) == 1)						      \
    __asm__ __volatile__ ("movb %0,%%fs:(%1)" :				      \
			  : "q" (__value),				      \
			    "r" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else if (sizeof (__value) == 4)					      \
    __asm__ __volatile__ ("movl %k0,%%fs:(%1)" :			      \
			  : "r" (__value),				      \
			    "r" (offsetof (struct _pthread_descr_struct,      \
					   member)));			      \
  else									      \
    {									      \
      if (sizeof (__value) != 8)					      \
	/* There should not be any value with a size other than 1, 4 or 8.  */\
	abort ();							      \
									      \
      __asm__ __volatile__ ("movq %0,%%fs:(%1)"	:			      \
			    : "r" (__value),				      \
			      "r" (offsetof (struct _pthread_descr_struct,    \
					     member)));			      \
    }									      \
})

#endif /* !__ASSEMBLER__ */

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS	1

/* Maximum size of the stack if the rlimit is unlimited.  */
#define ARCH_STACK_MAX_SIZE	32*1024*1024

/* The ia32e really want some help to prevent overheating.  */
#define BUSY_WAIT_NOP	__asm__ ("rep; nop")

#endif /* pt-machine.h */
